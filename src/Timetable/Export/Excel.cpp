// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Logging.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "UI.hpp"
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <format.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <workbook.h>
#include <worksheet.h>

bool printError = false;

void WriteXlsxTemplate(lxw_workbook* workbook, lxw_worksheet* worksheet, int cellWidth = 1)
{
    lxw_format* centerFormat = workbook_add_format(workbook);
    format_set_align(centerFormat, LXW_ALIGN_CENTER);

    lxw_format* lessonNumberFormat = workbook_add_format(workbook);
    format_set_font_size(lessonNumberFormat, 18);
    format_set_align(lessonNumberFormat, LXW_ALIGN_CENTER);
    format_set_align(lessonNumberFormat, LXW_ALIGN_VERTICAL_CENTER);

    // Write week days
    worksheet_set_column(worksheet, 1, settings.daysPerWeek * cellWidth, 15, NULL);
    for (size_t i = 0; i < settings.daysPerWeek; i++)
    {
        int weekDay = i;
        while (weekDay >= 7) weekDay -= 7;
        if (cellWidth == 1)
            worksheet_write_string(worksheet, 1, i + 1, weekDays[weekDay].c_str(), centerFormat);
        else
            worksheet_merge_range(worksheet, 1, i * cellWidth + 1, 1, (i + 1) * cellWidth,
                                  weekDays[weekDay].c_str(), centerFormat);
    }

    // Write lesson numbers
    for (size_t i = 0; i < settings.lessonsPerDay; i++)
    {
        worksheet_write_number(worksheet, i + 2, 0, i, lessonNumberFormat);
    }
}

void PrintXlsxError(lxw_workbook* workbook, lxw_worksheet* worksheet, int cellWidth = 1)
{
    lxw_format* errorFormat = workbook_add_format(workbook);
    format_set_font_size(errorFormat, 32);
    format_set_align(errorFormat, LXW_ALIGN_CENTER);
    format_set_align(errorFormat, LXW_ALIGN_VERTICAL_CENTER);
    format_set_font_color(errorFormat, LXW_COLOR_RED);
    format_set_text_wrap(errorFormat);

    std::string errorMessage =
        GetText("Press the Generate timetable button\nbefore exporting the timetable!");
    worksheet_merge_range(worksheet, 2, 1, settings.lessonsPerDay + 1,
                          settings.daysPerWeek * cellWidth, errorMessage.c_str(), errorFormat);
}

void ExportClassesAsXlsx(Timetable& timetable)
{
    LogInfo("Exporting classes of timetables/" + timetable.name + ".json");
    std::string fileName = "timetables/" + GetText("Classes") + "_" + timetable.name + ".xlsx";
    lxw_workbook* workbook = workbook_new(fileName.c_str());

    lxw_format* headingFormat = workbook_add_format(workbook);
    format_set_bold(headingFormat);
    format_set_font_size(headingFormat, 26);
    format_set_align(headingFormat, LXW_ALIGN_CENTER);
    format_set_align(headingFormat, LXW_ALIGN_VERTICAL_CENTER);

    lxw_format* centerFormat = workbook_add_format(workbook);
    format_set_align(centerFormat, LXW_ALIGN_CENTER);

    lxw_format* lessonFormat = workbook_add_format(workbook);
    format_set_align(lessonFormat, LXW_ALIGN_CENTER);
    format_set_text_wrap(lessonFormat);

    for (auto& classPair: timetable.classes)
    {
        LogInfo("Exporting class with id " + std::to_string(classPair.first));
        // Find longest combined lesson
        size_t longestCombinedLesson = 1;
        for (auto& lesson: classPair.second.timetableLessons)
        {
            if (lesson.second.lessonTeacherPairs.size() > longestCombinedLesson)
                longestCombinedLesson = lesson.second.lessonTeacherPairs.size();
        }
        lxw_worksheet* worksheet = workbook_add_worksheet(
            workbook, (classPair.second.number + classPair.second.letter).c_str());

        // Write class name
        worksheet_merge_range(worksheet, 0, 1, 0, longestCombinedLesson * settings.daysPerWeek,
                              (classPair.second.number + classPair.second.letter).c_str(),
                              headingFormat);

        // Write class teacher name
        worksheet_set_column(worksheet, 0, 0, 20, NULL);
        if (classPair.second.teacherId >= 0)
        {
            worksheet_write_string(
                worksheet, 0, 0,
                ("Class teacher:\n" + timetable.teachers[classPair.second.teacherId].name).c_str(),
                NULL);
        }

        // Write the template
        WriteXlsxTemplate(workbook, worksheet, longestCombinedLesson);

        // Write class timetable lessons
        classPair.second.days.resize(settings.daysPerWeek);
        for (size_t i = 0; i < settings.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(settings.lessonsPerDay);
            for (size_t j = 0; j < settings.lessonsPerDay; j++)
            {
                int timetableLessonId =
                    classPair.second.days[i].classroomLessonPairs[j].timetableLessonId;
                if (timetableLessonId < 0)
                {
                    worksheet_merge_range(worksheet, j + 2, i * longestCombinedLesson + 1, j + 2,
                                          (i + 1) * longestCombinedLesson, "", centerFormat);
                    continue;
                }
                if (j >= classPair.second.days[i].classroomLessonPairs.size()) continue;
                for (size_t k = 0;
                     k <
                     classPair.second.timetableLessons[timetableLessonId].lessonTeacherPairs.size();
                     k++)
                {
                    float cellMergeTemplate = longestCombinedLesson * 1.0f /
                                              classPair.second.timetableLessons[timetableLessonId]
                                                  .lessonTeacherPairs.size();
                    int cellMergeStart =
                        i * longestCombinedLesson + 1 + floor(k * cellMergeTemplate);
                    int cellMergeEnd =
                        i * longestCombinedLesson + floor((k + 1) * cellMergeTemplate);
                    LessonTeacherPair& lessonTeacherPair =
                        classPair.second.timetableLessons[timetableLessonId].lessonTeacherPairs[k];
                    int lessonId = lessonTeacherPair.lessonId;
                    int teacherId = lessonTeacherPair.teacherId;
                    int classroomId =
                        classPair.second.days[i].classroomLessonPairs[j].classroomIds[k];
                    std::string& lessonName = timetable.lessons[lessonId].name;
                    std::string& teacherName = timetable.teachers[teacherId].name;
                    std::string& classroomName = timetable.classrooms[classroomId].name;
                    std::string timetableLessonText =
                        lessonName + "\n" + teacherName + "\n" + classroomName;
                    if (cellMergeStart == cellMergeEnd)
                        worksheet_write_string(worksheet, j + 2, cellMergeStart,
                                               timetableLessonText.c_str(), lessonFormat);
                    else
                        worksheet_merge_range(worksheet, j + 2, cellMergeStart, j + 2, cellMergeEnd,
                                              timetableLessonText.c_str(), lessonFormat);
                }
            }
        }

        if (printError) PrintXlsxError(workbook, worksheet, longestCombinedLesson);
    }

    workbook_close(workbook);
}

struct TeacherData
{
    int classId = -1;
    int lessonId = -1;
    int classroomId = -1;
};

std::unordered_map<int, std::vector<TeacherData>> GetTeacherData(Timetable& timetable)
{
    std::unordered_map<int, std::vector<TeacherData>> teacherData;
    for (auto& teacher: timetable.teachers)
    {
        teacherData[teacher.first].resize(settings.daysPerWeek * settings.lessonsPerDay);
    }
    for (auto& classPair: timetable.classes)
    {
        classPair.second.days.resize(settings.daysPerWeek);
        for (size_t i = 0; i < settings.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(settings.lessonsPerDay);
            for (size_t j = 0; j < settings.lessonsPerDay; j++)
            {
                int timetableLessonId =
                    classPair.second.days[i].classroomLessonPairs[j].timetableLessonId;
                if (timetableLessonId < 0) continue;
                for (size_t k = 0;
                     k <
                     classPair.second.timetableLessons[timetableLessonId].lessonTeacherPairs.size();
                     k++)
                {
                    auto& lessonTeacherPair =
                        classPair.second.timetableLessons[timetableLessonId].lessonTeacherPairs[k];
                    int teacherId = lessonTeacherPair.teacherId;
                    auto& data = teacherData[teacherId][i * settings.lessonsPerDay + j];
                    if (data.lessonId > 0)
                    {
                        int weekDay = i;
                        while (weekDay >= 7) weekDay -= 7;
                        LogError("Teacher " + timetable.teachers[teacherId].name +
                                 " already has lesson " + timetable.lessons[data.lessonId].name +
                                 " with class " + timetable.classes[data.classId].number +
                                 timetable.classes[data.classId].letter + " in classroom " +
                                 timetable.classrooms[data.lessonId].name + " on " +
                                 weekDays[weekDay] + " at lesson number " + std::to_string(j));
                    }
                    data.lessonId = lessonTeacherPair.lessonId;
                    data.classId = classPair.first;
                    data.classroomId =
                        classPair.second.days[i].classroomLessonPairs[j].classroomIds[k];
                }
            }
        }
    }
    return teacherData;
}

void ExportTeachersAsXlsx(Timetable& timetable)
{
    LogInfo("Exporting teachers of timetables/" + timetable.name + ".json");
    std::string fileName = "timetables/" + GetText("Teachers") + "_" + timetable.name + ".xlsx";
    lxw_workbook* workbook = workbook_new(fileName.c_str());

    lxw_format* headingFormat = workbook_add_format(workbook);
    format_set_bold(headingFormat);
    format_set_font_size(headingFormat, 26);
    format_set_align(headingFormat, LXW_ALIGN_CENTER);
    format_set_align(headingFormat, LXW_ALIGN_VERTICAL_CENTER);

    lxw_format* centerFormat = workbook_add_format(workbook);
    format_set_align(centerFormat, LXW_ALIGN_CENTER);

    lxw_format* lessonFormat = workbook_add_format(workbook);
    format_set_align(lessonFormat, LXW_ALIGN_CENTER);
    format_set_text_wrap(lessonFormat);

    for (auto& teacher: timetable.teachers)
    {
        LogInfo("Exporting teacher with id " + std::to_string(teacher.first));
        lxw_worksheet* worksheet = workbook_add_worksheet(workbook, teacher.second.name.c_str());

        // Write teacher name
        worksheet_merge_range(worksheet, 0, 0, 0, settings.daysPerWeek, teacher.second.name.c_str(),
                              headingFormat);

        // Write the template
        WriteXlsxTemplate(workbook, worksheet);

        // Write teacher lessons
        auto teacherData = GetTeacherData(timetable);
        for (size_t i = 0; i < settings.daysPerWeek; i++)
        {
            for (size_t j = 0; j < settings.lessonsPerDay; j++)
            {
                int lessonId = teacherData[teacher.first][i * settings.lessonsPerDay + j].lessonId;
                if (lessonId < 0) continue;
                int classId = teacherData[teacher.first][i * settings.lessonsPerDay + j].classId;
                int classroomId =
                    teacherData[teacher.first][i * settings.lessonsPerDay + j].classroomId;
                std::string& lessonName = timetable.lessons[lessonId].name;
                std::string className =
                    timetable.classes[classId].number + timetable.classes[classId].letter;
                std::string& classroomName = timetable.classrooms[classroomId].name;
                std::string lessonText = lessonName + "\n" + className + "\n" + classroomName;
                worksheet_write_string(worksheet, j + 2, i + 1, lessonText.c_str(), lessonFormat);
            }
        }

        if (printError) PrintXlsxError(workbook, worksheet);
    }

    workbook_close(workbook);
}

struct ClassroomData
{
    int classId = -1;
    int lessonId = -1;
    int teacherId = -1;
};

std::unordered_map<int, std::vector<ClassroomData>> GetClassroomData(Timetable& timetable)
{
    std::unordered_map<int, std::vector<ClassroomData>> classroomData;
    for (auto& classroom: timetable.classrooms)
    {
        classroomData[classroom.first].resize(settings.daysPerWeek * settings.lessonsPerDay);
    }
    for (auto& classPair: timetable.classes)
    {
        classPair.second.days.resize(settings.daysPerWeek);
        for (size_t i = 0; i < settings.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(settings.lessonsPerDay);
            for (size_t j = 0; j < settings.lessonsPerDay; j++)
            {
                int timetableLessonId =
                    classPair.second.days[i].classroomLessonPairs[j].timetableLessonId;
                if (timetableLessonId < 0) continue;
                for (size_t k = 0;
                     k <
                     classPair.second.timetableLessons[timetableLessonId].lessonTeacherPairs.size();
                     k++)
                {
                    LessonTeacherPair& lessonTeacherPair =
                        classPair.second.timetableLessons[timetableLessonId].lessonTeacherPairs[k];
                    if (lessonTeacherPair.lessonId < 0) continue;
                    int classroomId =
                        classPair.second.days[i].classroomLessonPairs[j].classroomIds[k];
                    auto& data = classroomData[classroomId][i * settings.lessonsPerDay + j];
                    if (data.lessonId > 0)
                    {
                        int weekDay = i;
                        while (weekDay >= 7) weekDay -= 7;
                        LogError("Teacher " + timetable.classrooms[classroomId].name +
                                 " already has lesson " + timetable.lessons[data.lessonId].name +
                                 " by teacher " + timetable.teachers[data.teacherId].name +
                                 " with class " + timetable.classes[data.classId].number +
                                 timetable.classes[data.classId].letter + " on " +
                                 weekDays[weekDay] + " at lesson number " + std::to_string(j));
                    }
                    data.lessonId = lessonTeacherPair.lessonId;
                    data.classId = classPair.first;
                    data.teacherId = lessonTeacherPair.teacherId;
                }
            }
        }
    }
    return classroomData;
}

void ExportClassroomsAsXlsx(Timetable& timetable)
{
    LogInfo("Exporting classrooms of timetables/" + timetable.name + ".json");
    std::string fileName = "timetables/" + GetText("Classrooms") + "_" + timetable.name + ".xlsx";
    lxw_workbook* workbook = workbook_new(fileName.c_str());

    lxw_format* headingFormat = workbook_add_format(workbook);
    format_set_bold(headingFormat);
    format_set_font_size(headingFormat, 26);
    format_set_align(headingFormat, LXW_ALIGN_CENTER);
    format_set_align(headingFormat, LXW_ALIGN_VERTICAL_CENTER);

    lxw_format* centerFormat = workbook_add_format(workbook);
    format_set_align(centerFormat, LXW_ALIGN_CENTER);

    lxw_format* lessonFormat = workbook_add_format(workbook);
    format_set_align(lessonFormat, LXW_ALIGN_CENTER);
    format_set_text_wrap(lessonFormat);

    for (auto& classroom: timetable.classrooms)
    {
        LogInfo("Exporting classroom with id " + std::to_string(classroom.first));
        lxw_worksheet* worksheet = workbook_add_worksheet(workbook, classroom.second.name.c_str());

        // Write classroom name
        worksheet_merge_range(worksheet, 0, 0, 0, settings.daysPerWeek,
                              classroom.second.name.c_str(), headingFormat);

        // Write the template
        WriteXlsxTemplate(workbook, worksheet);

        // Write classroom lessons
        auto classroomData = GetClassroomData(timetable);
        for (size_t i = 0; i < settings.daysPerWeek; i++)
        {
            for (size_t j = 0; j < settings.lessonsPerDay; j++)
            {
                int lessonId =
                    classroomData[classroom.first][i * settings.lessonsPerDay + j].lessonId;
                if (lessonId < 0) continue;
                int classId =
                    classroomData[classroom.first][i * settings.lessonsPerDay + j].classId;
                int teacherId =
                    classroomData[classroom.first][i * settings.lessonsPerDay + j].teacherId;
                std::string& lessonName = timetable.lessons[lessonId].name;
                std::string className =
                    timetable.classes[classId].number + timetable.classes[classId].letter;
                std::string& teacherName = timetable.teachers[teacherId].name;
                std::string lessonText = lessonName + "\n" + className + "\n" + teacherName;
                worksheet_write_string(worksheet, j + 2, i + 1, lessonText.c_str(), lessonFormat);
            }
        }

        if (printError) PrintXlsxError(workbook, worksheet);
    }

    workbook_close(workbook);
}

void Timetable::ExportAsXlsx()
{
    Timetable timetable;
    printError = false;
    if (std::filesystem::exists("timetables/" + name + ".json"))
    {
        timetable.Load("timetables/" + name + ".json");
        LogInfo("Exporting timetables/" + name + ".json");
    }
    else
    {
        timetable = *this;
        printError = true;
        LogInfo("Exporting templates/" + name + ".json");
    }

    ExportClassesAsXlsx(timetable);
    ExportTeachersAsXlsx(timetable);
    ExportClassroomsAsXlsx(timetable);

    LogInfo("Exported templates/" + name + " as timetables/*_" + name + ".xlsx");
}

// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Logging.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "UI.hpp"
#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <workbook.h>

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
    worksheet_set_column(worksheet, 1, daysPerWeek * cellWidth, 15, NULL);
    for (size_t i = 0; i < daysPerWeek; i++)
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
    for (size_t i = 0; i < lessonsPerDay; i++)
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
    worksheet_merge_range(worksheet, 2, 1, lessonsPerDay + 1, daysPerWeek * cellWidth,
                          errorMessage.c_str(), errorFormat);
}

void Timetable::ExportClassesAsXlsx()
{
    LogInfo("Exporting classes of timetables/" + name + ".json");
    std::string fileName = "timetables/" + GetText("Classes") + "_" + name + ".xlsx";
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

    for (auto& classPair: classes)
    {
        LogInfo("Exporting class with id " + std::to_string(classPair.first));
        // Find longest combined lesson
        int longestCombinedLesson = 1;
        for (auto& lesson: classPair.second.timetableLessons)
        {
            if (lesson.second.lessonTeacherPairs.size() > (size_t)longestCombinedLesson)
                longestCombinedLesson = lesson.second.lessonTeacherPairs.size();
        }
        lxw_worksheet* worksheet = workbook_add_worksheet(
            workbook, (classPair.second.number + classPair.second.letter).c_str());

        // Write class name
        worksheet_merge_range(worksheet, 0, 1, 0, longestCombinedLesson * daysPerWeek,
                              (classPair.second.number + classPair.second.letter).c_str(),
                              headingFormat);

        // Write class teacher name
        worksheet_set_column(worksheet, 0, 0, 20, NULL);
        if (classPair.second.teacherId >= 0)
        {
            worksheet_write_string(
                worksheet, 0, 0,
                ("Class teacher:\n" + teachers[classPair.second.teacherId].name).c_str(), NULL);
        }

        // Write the template
        WriteXlsxTemplate(workbook, worksheet, longestCombinedLesson);

        // Write class timetable lessons
        classPair.second.days.resize(daysPerWeek);
        for (size_t i = 0; i < daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(lessonsPerDay);
            for (size_t j = 0; j < lessonsPerDay; j++)
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
                    std::string& lessonName = lessons[lessonId].name;
                    std::string& teacherName = teachers[teacherId].name;
                    std::string& classroomName = classrooms[classroomId].name;
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
        teacherData[teacher.first].resize(daysPerWeek * lessonsPerDay);
    }
    for (auto& classPair: timetable.classes)
    {
        classPair.second.days.resize(daysPerWeek);
        for (size_t i = 0; i < daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(lessonsPerDay);
            for (size_t j = 0; j < lessonsPerDay; j++)
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
                    int teacherId = lessonTeacherPair.teacherId;
                    if (teacherData[teacherId][i * lessonsPerDay + j].lessonId > 0)
                    {
                        int weekDay = i;
                        while (weekDay >= 7) weekDay -= 7;
                        std::cout
                            << "Error: teacher " << timetable.teachers[teacherId].name
                            << " already has lesson "
                            << timetable
                                   .lessons[teacherData[teacherId][i * lessonsPerDay + j].lessonId]
                                   .name
                            << " with class "
                            << timetable
                                   .classes[teacherData[teacherId][i * lessonsPerDay + j].classId]
                                   .number
                            << timetable
                                   .classes[teacherData[teacherId][i * lessonsPerDay + j].classId]
                                   .letter
                            << " in classroom "
                            << timetable
                                   .classrooms[teacherData[teacherId][i * lessonsPerDay + j]
                                                   .lessonId]
                                   .name
                            << " on " << weekDays[weekDay] << " at lesson number " << j << "!\n";
                    }
                    teacherData[teacherId][i * lessonsPerDay + j].lessonId =
                        lessonTeacherPair.lessonId;
                    teacherData[teacherId][i * lessonsPerDay + j].classId = classPair.first;
                    teacherData[teacherId][i * lessonsPerDay + j].classroomId =
                        classPair.second.days[i].classroomLessonPairs[j].classroomIds[k];
                }
            }
        }
    }
    return teacherData;
}

void Timetable::ExportTeachersAsXlsx()
{
    LogInfo("Exporting teachers of timetables/" + name + ".json");
    std::string fileName = "timetables/" + GetText("Teachers") + "_" + name + ".xlsx";
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

    for (auto& teacher: teachers)
    {
        LogInfo("Exporting teacher with id " + std::to_string(teacher.first));
        lxw_worksheet* worksheet = workbook_add_worksheet(workbook, teacher.second.name.c_str());

        // Write teacher name
        worksheet_merge_range(worksheet, 0, 0, 0, daysPerWeek, teacher.second.name.c_str(),
                              headingFormat);

        // Write the template
        WriteXlsxTemplate(workbook, worksheet);

        // Write teacher lessons
        auto teacherData = GetTeacherData(*this);
        for (size_t i = 0; i < daysPerWeek; i++)
        {
            for (size_t j = 0; j < lessonsPerDay; j++)
            {
                int lessonId = teacherData[teacher.first][i * lessonsPerDay + j].lessonId;
                if (lessonId < 0) continue;
                int classId = teacherData[teacher.first][i * lessonsPerDay + j].classId;
                int classroomId = teacherData[teacher.first][i * lessonsPerDay + j].classroomId;
                std::string& lessonName = lessons[lessonId].name;
                std::string className = classes[classId].number + classes[classId].letter;
                std::string& classroomName = classrooms[classroomId].name;
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
        classroomData[classroom.first].resize(daysPerWeek * lessonsPerDay);
    }
    for (auto& classPair: timetable.classes)
    {
        classPair.second.days.resize(daysPerWeek);
        for (size_t i = 0; i < daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(lessonsPerDay);
            for (size_t j = 0; j < lessonsPerDay; j++)
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
                    if (classroomData[classroomId][i * lessonsPerDay + j].lessonId > 0)
                    {
                        int weekDay = i;
                        while (weekDay >= 7) weekDay -= 7;
                        std::cout << "Error: teacher " << timetable.classrooms[classroomId].name
                                  << " already has lesson "
                                  << timetable
                                         .lessons[classroomData[classroomId][i * lessonsPerDay + j]
                                                      .lessonId]
                                         .name
                                  << " by teacher "
                                  << timetable
                                         .teachers[classroomData[classroomId][i * lessonsPerDay + j]
                                                       .teacherId]
                                         .name
                                  << " with class "
                                  << timetable
                                         .classes[classroomData[classroomId][i * lessonsPerDay + j]
                                                      .classId]
                                         .number
                                  << timetable
                                         .classes[classroomData[classroomId][i * lessonsPerDay + j]
                                                      .classId]
                                         .letter
                                  << " on " << weekDays[weekDay] << " at lesson number " << j
                                  << "!\n";
                    }
                    classroomData[classroomId][i * lessonsPerDay + j].lessonId =
                        lessonTeacherPair.lessonId;
                    classroomData[classroomId][i * lessonsPerDay + j].classId = classPair.first;
                    classroomData[classroomId][i * lessonsPerDay + j].teacherId =
                        lessonTeacherPair.teacherId;
                }
            }
        }
    }
    return classroomData;
}

void Timetable::ExportClassroomsAsXlsx()
{
    LogInfo("Exporting classrooms of timetables/" + name + ".json");
    std::string fileName = "timetables/" + GetText("Classrooms") + "_" + name + ".xlsx";
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

    for (auto& classroom: classrooms)
    {
        LogInfo("Exporting classroom with id " + std::to_string(classroom.first));
        lxw_worksheet* worksheet = workbook_add_worksheet(workbook, classroom.second.name.c_str());

        // Write classroom name
        worksheet_merge_range(worksheet, 0, 0, 0, daysPerWeek, classroom.second.name.c_str(),
                              headingFormat);

        // Write the template
        WriteXlsxTemplate(workbook, worksheet);

        // Write classroom lessons
        auto classroomData = GetClassroomData(*this);
        for (size_t i = 0; i < daysPerWeek; i++)
        {
            for (size_t j = 0; j < lessonsPerDay; j++)
            {
                int lessonId = classroomData[classroom.first][i * lessonsPerDay + j].lessonId;
                if (lessonId < 0) continue;
                int classId = classroomData[classroom.first][i * lessonsPerDay + j].classId;
                int teacherId = classroomData[classroom.first][i * lessonsPerDay + j].teacherId;
                std::string& lessonName = lessons[lessonId].name;
                std::string className = classes[classId].number + classes[classId].letter;
                std::string& teacherName = teachers[teacherId].name;
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
    printError = false;
    if (std::filesystem::exists("timetables/" + name + ".json"))
    {
        Load("timetables/" + name + ".json");
        LogInfo("Exporting timetables/" + name + ".json");
    }
    else
    {
        printError = true;
        LogInfo("Exporting templates/" + name + ".json");
    }

    ExportClassesAsXlsx();
    ExportTeachersAsXlsx();
    ExportClassroomsAsXlsx();

    LogInfo("Exported templates/" + name + " as timetables/*_" + name + ".xlsx");
}

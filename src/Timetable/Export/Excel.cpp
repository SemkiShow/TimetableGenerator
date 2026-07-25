// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Logging.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "UI.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <format.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <workbook.h>
#include <worksheet.h>

static bool g_printError = false;

constexpr int LESSON_FONT_SIZE = 18;
constexpr int WEEK_DAYS_FONT_SIZE = 15;
constexpr int ERROR_FONT_SIZE = 32;
constexpr int HEADING_FONT_SIZE = 26;
constexpr int TEACHER_FONT_SIZE = 20;

static void WriteXlsxTemplate(lxw_workbook* workbook, lxw_worksheet* worksheet, int cellWidth = 1)
{
    lxw_format* centerFormat = workbook_add_format(workbook);
    format_set_align(centerFormat, LXW_ALIGN_CENTER);

    lxw_format* lessonNumberFormat = workbook_add_format(workbook);
    format_set_font_size(lessonNumberFormat, LESSON_FONT_SIZE);
    format_set_align(lessonNumberFormat, LXW_ALIGN_CENTER);
    format_set_align(lessonNumberFormat, LXW_ALIGN_VERTICAL_CENTER);

    // Write week days
    worksheet_set_column(worksheet, 1, g_settings.daysPerWeek * cellWidth, WEEK_DAYS_FONT_SIZE,
                         nullptr);
    for (int i = 0; i < g_settings.daysPerWeek; i++)
    {
        size_t weekDay = i;
        while (weekDay >= g_weekDays.size()) weekDay -= g_weekDays.size();
        if (cellWidth == 1)
            worksheet_write_string(worksheet, 1, i + 1, g_weekDays[weekDay], centerFormat);
        else
            worksheet_merge_range(worksheet, 1, i * cellWidth + 1, 1, (i + 1) * cellWidth,
                                  g_weekDays[weekDay], centerFormat);
    }

    // Write lesson numbers
    for (int i = 0; i < g_settings.lessonsPerDay; i++)
    {
        worksheet_write_number(worksheet, i + 2, 0, i, lessonNumberFormat);
    }
}

static void PrintXlsxError(lxw_workbook* workbook, lxw_worksheet* worksheet, int cellWidth = 1)
{
    lxw_format* errorFormat = workbook_add_format(workbook);
    format_set_font_size(errorFormat, ERROR_FONT_SIZE);
    format_set_align(errorFormat, LXW_ALIGN_CENTER);
    format_set_align(errorFormat, LXW_ALIGN_VERTICAL_CENTER);
    format_set_font_color(errorFormat, LXW_COLOR_RED);
    format_set_text_wrap(errorFormat);

    const char* errorMessage =
        gettext("Press the Generate timetable button\nbefore exporting the timetable!");
    worksheet_merge_range(worksheet, 2, 1, g_settings.lessonsPerDay + 1,
                          g_settings.daysPerWeek * cellWidth, errorMessage, errorFormat);
}

static void ExportClassesAsXlsx(Timetable& timetable)
{
    LogInfo("Exporting classes of timetables/%s.json", timetable.name.c_str());
    std::string fileName = "timetables/" + GetText("Classes") + "_" + timetable.name + ".xlsx";
    lxw_workbook* workbook = workbook_new(fileName.c_str());

    lxw_format* headingFormat = workbook_add_format(workbook);
    format_set_bold(headingFormat);
    format_set_font_size(headingFormat, HEADING_FONT_SIZE);
    format_set_align(headingFormat, LXW_ALIGN_CENTER);
    format_set_align(headingFormat, LXW_ALIGN_VERTICAL_CENTER);

    lxw_format* centerFormat = workbook_add_format(workbook);
    format_set_align(centerFormat, LXW_ALIGN_CENTER);

    lxw_format* lessonFormat = workbook_add_format(workbook);
    format_set_align(lessonFormat, LXW_ALIGN_CENTER);
    format_set_text_wrap(lessonFormat);

    for (auto& classPair: timetable.classes)
    {
        LogInfo("Exporting class with id %d", classPair.first);
        // Find longest combined lesson
        int longestCombinedLesson = 1;
        for (auto& lesson: classPair.second.timetableLessons)
        {
            longestCombinedLesson =
                std::max((int)lesson.second.lessonTeacherPairs.size(), longestCombinedLesson);
        }
        lxw_worksheet* worksheet = workbook_add_worksheet(
            workbook, (classPair.second.number + classPair.second.letter).c_str());

        // Write class name
        worksheet_merge_range(worksheet, 0, 1, 0, longestCombinedLesson * g_settings.daysPerWeek,
                              (classPair.second.number + classPair.second.letter).c_str(),
                              headingFormat);

        // Write class teacher name
        worksheet_set_column(worksheet, 0, 0, TEACHER_FONT_SIZE, nullptr);
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
        classPair.second.days.resize(g_settings.daysPerWeek);
        for (int i = 0; i < g_settings.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(g_settings.lessonsPerDay);
            for (int j = 0; j < g_settings.lessonsPerDay; j++)
            {
                int timetableLessonId =
                    classPair.second.days[i].classroomLessonPairs[j].timetableLessonId;
                if (timetableLessonId < 0)
                {
                    worksheet_merge_range(worksheet, j + 2, i * longestCombinedLesson + 1, j + 2,
                                          (i + 1) * longestCombinedLesson, "", centerFormat);
                    continue;
                }
                if (j >= (int)classPair.second.days[i].classroomLessonPairs.size()) continue;
                for (size_t k = 0;
                     k <
                     classPair.second.timetableLessons[timetableLessonId].lessonTeacherPairs.size();
                     k++)
                {
                    int cellMergeTemplate =
                        longestCombinedLesson /
                        (int)classPair.second.timetableLessons[timetableLessonId]
                            .lessonTeacherPairs.size();
                    int cellMergeStart = i * longestCombinedLesson + 1 + (int)k * cellMergeTemplate;
                    int cellMergeEnd = i * longestCombinedLesson + (int)(k + 1) * cellMergeTemplate;
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

        if (g_printError) PrintXlsxError(workbook, worksheet, longestCombinedLesson);
    }

    workbook_close(workbook);
}

struct TeacherData
{
    int classId = -1;
    int lessonId = -1;
    int classroomId = -1;
};

static std::unordered_map<int, std::vector<TeacherData>> GetTeacherData(Timetable& timetable)
{
    std::unordered_map<int, std::vector<TeacherData>> teacherData;
    for (auto& teacher: timetable.teachers)
    {
        teacherData[teacher.first].resize((size_t)g_settings.daysPerWeek *
                                          g_settings.lessonsPerDay);
    }
    for (auto& classPair: timetable.classes)
    {
        classPair.second.days.resize(g_settings.daysPerWeek);
        for (int i = 0; i < g_settings.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(g_settings.lessonsPerDay);
            for (int j = 0; j < g_settings.lessonsPerDay; j++)
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
                    auto& data = teacherData[teacherId][i * g_settings.lessonsPerDay + j];
                    if (data.lessonId > 0)
                    {
                        size_t weekDay = i;
                        while (weekDay >= g_weekDays.size()) weekDay -= g_weekDays.size();
                        LOG_ERROR(
                            "Teacher %s already has lesson %s with class %s%s in classroom %s on %s at lesson number %d",
                            timetable.teachers[teacherId].name.c_str(),
                            timetable.lessons[data.lessonId].name.c_str(),
                            timetable.classes[data.classId].number.c_str(),
                            timetable.classes[data.classId].letter.c_str(),
                            timetable.classrooms[data.lessonId].name.c_str(), g_weekDays[weekDay],
                            j);
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

static void ExportTeachersAsXlsx(Timetable& timetable)
{
    LogInfo("Exporting teachers of timetables/%s.json", timetable.name.c_str());
    std::string fileName = "timetables/" + GetText("Teachers") + "_" + timetable.name + ".xlsx";
    lxw_workbook* workbook = workbook_new(fileName.c_str());

    lxw_format* headingFormat = workbook_add_format(workbook);
    format_set_bold(headingFormat);
    format_set_font_size(headingFormat, HEADING_FONT_SIZE);
    format_set_align(headingFormat, LXW_ALIGN_CENTER);
    format_set_align(headingFormat, LXW_ALIGN_VERTICAL_CENTER);

    lxw_format* centerFormat = workbook_add_format(workbook);
    format_set_align(centerFormat, LXW_ALIGN_CENTER);

    lxw_format* lessonFormat = workbook_add_format(workbook);
    format_set_align(lessonFormat, LXW_ALIGN_CENTER);
    format_set_text_wrap(lessonFormat);

    for (auto& teacher: timetable.teachers)
    {
        LogInfo("Exporting teacher with id %d", teacher.first);
        lxw_worksheet* worksheet = workbook_add_worksheet(workbook, teacher.second.name.c_str());

        // Write teacher name
        worksheet_merge_range(worksheet, 0, 0, 0, g_settings.daysPerWeek,
                              teacher.second.name.c_str(), headingFormat);

        // Write the template
        WriteXlsxTemplate(workbook, worksheet);

        // Write teacher lessons
        auto teacherData = GetTeacherData(timetable);
        for (int i = 0; i < g_settings.daysPerWeek; i++)
        {
            for (int j = 0; j < g_settings.lessonsPerDay; j++)
            {
                int lessonId =
                    teacherData[teacher.first][i * g_settings.lessonsPerDay + j].lessonId;
                if (lessonId < 0) continue;
                int classId = teacherData[teacher.first][i * g_settings.lessonsPerDay + j].classId;
                int classroomId =
                    teacherData[teacher.first][i * g_settings.lessonsPerDay + j].classroomId;
                std::string& lessonName = timetable.lessons[lessonId].name;
                std::string className =
                    timetable.classes[classId].number + timetable.classes[classId].letter;
                std::string& classroomName = timetable.classrooms[classroomId].name;
                std::string lessonText = lessonName + "\n" + className + "\n" + classroomName;
                worksheet_write_string(worksheet, j + 2, i + 1, lessonText.c_str(), lessonFormat);
            }
        }

        if (g_printError) PrintXlsxError(workbook, worksheet);
    }

    workbook_close(workbook);
}

struct ClassroomData
{
    int classId = -1;
    int lessonId = -1;
    int teacherId = -1;
};

static std::unordered_map<int, std::vector<ClassroomData>> GetClassroomData(Timetable& timetable)
{
    std::unordered_map<int, std::vector<ClassroomData>> classroomData;
    for (auto& classroom: timetable.classrooms)
    {
        classroomData[classroom.first].resize((size_t)g_settings.daysPerWeek *
                                              g_settings.lessonsPerDay);
    }
    for (auto& classPair: timetable.classes)
    {
        classPair.second.days.resize(g_settings.daysPerWeek);
        for (int i = 0; i < g_settings.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(g_settings.lessonsPerDay);
            for (int j = 0; j < g_settings.lessonsPerDay; j++)
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
                    auto& data = classroomData[classroomId][i * g_settings.lessonsPerDay + j];
                    if (data.lessonId > 0)
                    {
                        size_t weekDay = i;
                        while (weekDay >= g_weekDays.size()) weekDay -= g_weekDays.size();
                        LOG_ERROR(
                            "Classroom %s already has lesson %s by teacher %s with class %s%s on %s at lesson number %d",
                            timetable.classrooms[classroomId].name.c_str(),
                            timetable.lessons[data.lessonId].name.c_str(),
                            timetable.teachers[data.teacherId].name.c_str(),
                            timetable.classes[data.classId].number.c_str(),
                            timetable.classes[data.classId].letter.c_str(), g_weekDays[weekDay], j);
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

static void ExportClassroomsAsXlsx(Timetable& timetable)
{
    LogInfo("Exporting classrooms of timetables/%s.json", timetable.name.c_str());
    std::string fileName = "timetables/" + GetText("Classrooms") + "_" + timetable.name + ".xlsx";
    lxw_workbook* workbook = workbook_new(fileName.c_str());

    lxw_format* headingFormat = workbook_add_format(workbook);
    format_set_bold(headingFormat);
    format_set_font_size(headingFormat, HEADING_FONT_SIZE);
    format_set_align(headingFormat, LXW_ALIGN_CENTER);
    format_set_align(headingFormat, LXW_ALIGN_VERTICAL_CENTER);

    lxw_format* centerFormat = workbook_add_format(workbook);
    format_set_align(centerFormat, LXW_ALIGN_CENTER);

    lxw_format* lessonFormat = workbook_add_format(workbook);
    format_set_align(lessonFormat, LXW_ALIGN_CENTER);
    format_set_text_wrap(lessonFormat);

    for (auto& classroom: timetable.classrooms)
    {
        LogInfo("Exporting classroom with id %d", classroom.first);
        lxw_worksheet* worksheet = workbook_add_worksheet(workbook, classroom.second.name.c_str());

        // Write classroom name
        worksheet_merge_range(worksheet, 0, 0, 0, g_settings.daysPerWeek,
                              classroom.second.name.c_str(), headingFormat);

        // Write the template
        WriteXlsxTemplate(workbook, worksheet);

        // Write classroom lessons
        auto classroomData = GetClassroomData(timetable);
        for (int i = 0; i < g_settings.daysPerWeek; i++)
        {
            for (int j = 0; j < g_settings.lessonsPerDay; j++)
            {
                int lessonId =
                    classroomData[classroom.first][i * g_settings.lessonsPerDay + j].lessonId;
                if (lessonId < 0) continue;
                int classId =
                    classroomData[classroom.first][i * g_settings.lessonsPerDay + j].classId;
                int teacherId =
                    classroomData[classroom.first][i * g_settings.lessonsPerDay + j].teacherId;
                std::string& lessonName = timetable.lessons[lessonId].name;
                std::string className =
                    timetable.classes[classId].number + timetable.classes[classId].letter;
                std::string& teacherName = timetable.teachers[teacherId].name;
                std::string lessonText = lessonName + "\n" + className + "\n" + teacherName;
                worksheet_write_string(worksheet, j + 2, i + 1, lessonText.c_str(), lessonFormat);
            }
        }

        if (g_printError) PrintXlsxError(workbook, worksheet);
    }

    workbook_close(workbook);
}

void Timetable::ExportAsXlsx() const
{
    Timetable timetable;
    g_printError = false;
    if (std::filesystem::exists("timetables/" + name + ".json"))
    {
        timetable.Load("timetables/" + name + ".json");
        LogInfo("Exporting timetables/%s.json", name.c_str());
    }
    else
    {
        timetable = *this;
        g_printError = true;
        LogInfo("Exporting templates/%s.json", name.c_str());
    }

    ExportClassesAsXlsx(timetable);
    ExportTeachersAsXlsx(timetable);
    ExportClassroomsAsXlsx(timetable);

    LogInfo("Exported templates/%s as timetables/*_%s.xlsx", name.c_str(), name.c_str());
}

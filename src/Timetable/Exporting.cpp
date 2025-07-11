#include "Logging.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"
#include "UI.hpp"
#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <xlsxwriter.h>

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
    for (int i = 0; i < daysPerWeek; i++)
    {
        int weekDay = i;
        while (weekDay >= 7)
            weekDay -= 7;
        if (cellWidth == 1)
            worksheet_write_string(worksheet, 1, i + 1, weekDays[weekDay].c_str(), centerFormat);
        else
            worksheet_merge_range(worksheet, 1, i * cellWidth + 1, 1, (i + 1) * cellWidth,
                                  weekDays[weekDay].c_str(), centerFormat);
    }

    // Write lesson numbers
    for (int i = 0; i < lessonsPerDay; i++)
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

    std::string errorMessage = labels["Press the Generate timetable button"] + '\n' +
                               labels["before exporting the timetable!"];
    worksheet_merge_range(worksheet, 2, 1, lessonsPerDay + 1, daysPerWeek * cellWidth,
                          errorMessage.c_str(), errorFormat);
}

void ExportClassesAsXlsx(Timetable* timetable)
{
    LogInfo("Exporting classes of timetables/" + timetable->name + ".json");
    std::string fileName = "timetables/classes_" + timetable->name + ".xlsx";
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

    for (auto& classPair: timetable->classes)
    {
        LogInfo("Exporting class with ID " + std::to_string(classPair.first));
        // Find longest combined lesson
        int longestCombinedLesson = 1;
        for (auto& lesson: classPair.second.timetableLessons)
        {
            if (lesson.second.lessonTeacherPairs.size() > longestCombinedLesson)
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
        if (classPair.second.teacherID >= 0)
        {
            worksheet_write_string(
                worksheet, 0, 0,
                ("Class teacher:\n" + timetable->teachers[classPair.second.teacherID].name).c_str(),
                NULL);
        }

        // Write the template
        WriteXlsxTemplate(workbook, worksheet, longestCombinedLesson);

        // Write class timetable lessons
        classPair.second.days.resize(daysPerWeek);
        for (int i = 0; i < daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(lessonsPerDay);
            for (int j = 0; j < lessonsPerDay; j++)
            {
                int timetableLessonID =
                    classPair.second.days[i].classroomLessonPairs[j].timetableLessonID;
                if (timetableLessonID < 0)
                {
                    worksheet_merge_range(worksheet, j + 2, i * longestCombinedLesson + 1, j + 2,
                                          (i + 1) * longestCombinedLesson, "", centerFormat);
                    continue;
                }
                if (j >= classPair.second.days[i].classroomLessonPairs.size()) continue;
                for (int k = 0;
                     k <
                     classPair.second.timetableLessons[timetableLessonID].lessonTeacherPairs.size();
                     k++)
                {
                    float cellMergeTemplate = longestCombinedLesson * 1.0f /
                                              classPair.second.timetableLessons[timetableLessonID]
                                                  .lessonTeacherPairs.size();
                    int cellMergeStart =
                        i * longestCombinedLesson + 1 + floor(k * cellMergeTemplate);
                    int cellMergeEnd =
                        i * longestCombinedLesson + floor((k + 1) * cellMergeTemplate);
                    LessonTeacherPair& lessonTeacherPair =
                        classPair.second.timetableLessons[timetableLessonID].lessonTeacherPairs[k];
                    int lessonID = lessonTeacherPair.lessonID;
                    int teacherID = lessonTeacherPair.teacherID;
                    int classroomID =
                        classPair.second.days[i].classroomLessonPairs[j].classroomIDs[k];
                    std::string& lessonName = timetable->lessons[lessonID].name;
                    std::string& teacherName = timetable->teachers[teacherID].name;
                    std::string& classroomName = timetable->classrooms[classroomID].name;
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
    int classID = -1;
    int lessonID = -1;
    int classroomID = -1;
};

std::unordered_map<int, std::vector<TeacherData>> GetTeacherData(Timetable* timetable)
{
    std::unordered_map<int, std::vector<TeacherData>> teacherData;
    for (auto& teacher: timetable->teachers)
    {
        teacherData[teacher.first].resize(daysPerWeek * lessonsPerDay);
    }
    for (auto& classPair: timetable->classes)
    {
        classPair.second.days.resize(daysPerWeek);
        for (int i = 0; i < daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(lessonsPerDay);
            for (int j = 0; j < lessonsPerDay; j++)
            {
                int timetableLessonID =
                    classPair.second.days[i].classroomLessonPairs[j].timetableLessonID;
                if (timetableLessonID < 0) continue;
                for (int k = 0;
                     k <
                     classPair.second.timetableLessons[timetableLessonID].lessonTeacherPairs.size();
                     k++)
                {
                    LessonTeacherPair& lessonTeacherPair =
                        classPair.second.timetableLessons[timetableLessonID].lessonTeacherPairs[k];
                    int teacherID = lessonTeacherPair.teacherID;
                    if (teacherData[teacherID][i * lessonsPerDay + j].lessonID > 0)
                    {
                        int weekDay = i;
                        while (weekDay >= 7)
                            weekDay -= 7;
                        std::cout
                            << "Error: teacher " << timetable->teachers[teacherID].name
                            << " already has lesson "
                            << timetable
                                   ->lessons[teacherData[teacherID][i * lessonsPerDay + j].lessonID]
                                   .name
                            << " with class "
                            << timetable
                                   ->classes[teacherData[teacherID][i * lessonsPerDay + j].classID]
                                   .number
                            << timetable
                                   ->classes[teacherData[teacherID][i * lessonsPerDay + j].classID]
                                   .letter
                            << " in classroom "
                            << timetable
                                   ->classrooms[teacherData[teacherID][i * lessonsPerDay + j]
                                                    .lessonID]
                                   .name
                            << " on " << weekDays[weekDay] << " at lesson number " << j << "!\n";
                    }
                    teacherData[teacherID][i * lessonsPerDay + j].lessonID =
                        lessonTeacherPair.lessonID;
                    teacherData[teacherID][i * lessonsPerDay + j].classID = classPair.first;
                    teacherData[teacherID][i * lessonsPerDay + j].classroomID =
                        classPair.second.days[i].classroomLessonPairs[j].classroomIDs[k];
                }
            }
        }
    }
    return teacherData;
}

void ExportTeachersAsXlsx(Timetable* timetable)
{
    LogInfo("Exporting teachers of timetables/" + timetable->name + ".json");
    std::string fileName = "timetables/teachers_" + timetable->name + ".xlsx";
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

    for (auto& teacher: timetable->teachers)
    {
        LogInfo("Exporting teacher with ID " + std::to_string(teacher.first));
        lxw_worksheet* worksheet = workbook_add_worksheet(workbook, teacher.second.name.c_str());

        // Write teacher name
        worksheet_merge_range(worksheet, 0, 0, 0, daysPerWeek, teacher.second.name.c_str(),
                              headingFormat);

        // Write the template
        WriteXlsxTemplate(workbook, worksheet);

        // Write teacher lessons
        auto teacherData = GetTeacherData(timetable);
        for (int i = 0; i < daysPerWeek; i++)
        {
            for (int j = 0; j < lessonsPerDay; j++)
            {
                int lessonID = teacherData[teacher.first][i * lessonsPerDay + j].lessonID;
                if (lessonID < 0) continue;
                int classID = teacherData[teacher.first][i * lessonsPerDay + j].classID;
                int classroomID = teacherData[teacher.first][i * lessonsPerDay + j].classroomID;
                std::string& lessonName = timetable->lessons[lessonID].name;
                std::string className =
                    timetable->classes[classID].number + timetable->classes[classID].letter;
                std::string& classroomName = timetable->classrooms[classroomID].name;
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
    int classID = -1;
    int lessonID = -1;
    int teacherID = -1;
};

std::unordered_map<int, std::vector<ClassroomData>> GetClassroomData(Timetable* timetable)
{
    std::unordered_map<int, std::vector<ClassroomData>> classroomData;
    for (auto& classroom: timetable->classrooms)
    {
        classroomData[classroom.first].resize(daysPerWeek * lessonsPerDay);
    }
    for (auto& classPair: timetable->classes)
    {
        classPair.second.days.resize(daysPerWeek);
        for (int i = 0; i < daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(lessonsPerDay);
            for (int j = 0; j < lessonsPerDay; j++)
            {
                int timetableLessonID =
                    classPair.second.days[i].classroomLessonPairs[j].timetableLessonID;
                if (timetableLessonID < 0) continue;
                for (int k = 0;
                     k <
                     classPair.second.timetableLessons[timetableLessonID].lessonTeacherPairs.size();
                     k++)
                {
                    LessonTeacherPair& lessonTeacherPair =
                        classPair.second.timetableLessons[timetableLessonID].lessonTeacherPairs[k];
                    if (lessonTeacherPair.lessonID < 0) continue;
                    int classroomID =
                        classPair.second.days[i].classroomLessonPairs[j].classroomIDs[k];
                    if (classroomData[classroomID][i * lessonsPerDay + j].lessonID > 0)
                    {
                        int weekDay = i;
                        while (weekDay >= 7)
                            weekDay -= 7;
                        std::cout
                            << "Error: teacher " << timetable->classrooms[classroomID].name
                            << " already has lesson "
                            << timetable
                                   ->lessons[classroomData[classroomID][i * lessonsPerDay + j]
                                                 .lessonID]
                                   .name
                            << " by teacher "
                            << timetable
                                   ->teachers[classroomData[classroomID][i * lessonsPerDay + j]
                                                  .teacherID]
                                   .name
                            << " with class "
                            << timetable
                                   ->classes[classroomData[classroomID][i * lessonsPerDay + j]
                                                 .classID]
                                   .number
                            << timetable
                                   ->classes[classroomData[classroomID][i * lessonsPerDay + j]
                                                 .classID]
                                   .letter
                            << " on " << weekDays[weekDay] << " at lesson number " << j << "!\n";
                    }
                    classroomData[classroomID][i * lessonsPerDay + j].lessonID =
                        lessonTeacherPair.lessonID;
                    classroomData[classroomID][i * lessonsPerDay + j].classID = classPair.first;
                    classroomData[classroomID][i * lessonsPerDay + j].teacherID =
                        lessonTeacherPair.teacherID;
                }
            }
        }
    }
    return classroomData;
}

void ExportClassroomsAsXlsx(Timetable* timetable)
{
    LogInfo("Exporting classrooms of timetables/" + timetable->name + ".json");
    std::string fileName = "timetables/classrooms_" + timetable->name + ".xlsx";
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

    for (auto& classroom: timetable->classrooms)
    {
        LogInfo("Exporting classroom with ID " + std::to_string(classroom.first));
        lxw_worksheet* worksheet = workbook_add_worksheet(workbook, classroom.second.name.c_str());

        // Write classroom name
        worksheet_merge_range(worksheet, 0, 0, 0, daysPerWeek, classroom.second.name.c_str(),
                              headingFormat);

        // Write the template
        WriteXlsxTemplate(workbook, worksheet);

        // Write classroom lessons
        auto classroomData = GetClassroomData(timetable);
        for (int i = 0; i < daysPerWeek; i++)
        {
            for (int j = 0; j < lessonsPerDay; j++)
            {
                int lessonID = classroomData[classroom.first][i * lessonsPerDay + j].lessonID;
                if (lessonID < 0) continue;
                int classID = classroomData[classroom.first][i * lessonsPerDay + j].classID;
                int teacherID = classroomData[classroom.first][i * lessonsPerDay + j].teacherID;
                std::string& lessonName = timetable->lessons[lessonID].name;
                std::string className =
                    timetable->classes[classID].number + timetable->classes[classID].letter;
                std::string& teacherName = timetable->teachers[teacherID].name;
                std::string lessonText = lessonName + "\n" + className + "\n" + teacherName;
                worksheet_write_string(worksheet, j + 2, i + 1, lessonText.c_str(), lessonFormat);
            }
        }

        if (printError) PrintXlsxError(workbook, worksheet);
    }

    workbook_close(workbook);
}

void ExportTimetableAsXlsx(Timetable* timetable)
{
    Timetable exportTimetable;
    printError = false;
    if (std::filesystem::exists("timetables/" + timetable->name + ".json"))
    {
        LoadTimetable("timetables/" + timetable->name + ".json", &exportTimetable);
        LogInfo("Exporting timetables/" + timetable->name + ".json");
    }
    else
    {
        exportTimetable = *timetable;
        printError = true;
        LogInfo("Exporting templates/" + timetable->name + ".json");
    }

    ExportClassesAsXlsx(&exportTimetable);
    ExportTeachersAsXlsx(&exportTimetable);
    ExportClassroomsAsXlsx(&exportTimetable);

    std::cout << "Exported templates/" << timetable->name << " as timetables/*_" << timetable->name
              << ".xlsx\n";
    LogInfo("Exported templates/" + timetable->name + " as timetables/*_" + timetable->name +
            ".xlsx");
}

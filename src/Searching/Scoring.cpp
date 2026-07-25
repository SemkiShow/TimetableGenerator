// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Logging.hpp"
#include "Searching.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"
#include <algorithm>
#include <cstddef>
#include <set>
#include <unordered_map>
#include <vector>

static std::unordered_map<int, std::vector<WorkDay>> GetTeacherLessons(Timetable& timetable)
{
    std::unordered_map<int, std::vector<WorkDay>> teacherLessons;
    for (auto& teacher: timetable.teachers)
    {
        teacherLessons[teacher.first].resize(g_iterationData.daysPerWeek);
        for (int i = 0; i < g_iterationData.daysPerWeek; i++)
        {
            teacherLessons[teacher.first][i].lessonIds.resize(g_iterationData.lessonsPerDay,
                                                              NO_LESSON);
        }
    }
    for (auto& classPair: timetable.classes)
    {
        classPair.second.days.resize(g_iterationData.daysPerWeek);
        for (int i = 0; i < g_iterationData.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(g_iterationData.lessonsPerDay);
            for (int j = 0; j < g_iterationData.lessonsPerDay; j++)
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
                    teacherLessons[lessonTeacherPair.teacherId][i].lessonIds[j] =
                        lessonTeacherPair.lessonId;
                }
            }
        }
    }
    return teacherLessons;
}

// I use lessonIds here in place of non-existent classroomIds
static std::unordered_map<int, std::vector<WorkDay>> GetTeacherClassrooms(Timetable& timetable)
{
    std::unordered_map<int, std::vector<WorkDay>> teacherClassrooms;
    for (auto& teacher: timetable.teachers)
    {
        teacherClassrooms[teacher.first].resize(g_iterationData.daysPerWeek);
        for (int i = 0; i < g_iterationData.daysPerWeek; i++)
        {
            teacherClassrooms[teacher.first][i].lessonIds.resize(g_iterationData.lessonsPerDay,
                                                                 NO_LESSON);
        }
    }
    for (auto& classPair: timetable.classes)
    {
        classPair.second.days.resize(g_iterationData.daysPerWeek);
        for (int i = 0; i < g_iterationData.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(g_iterationData.lessonsPerDay);
            for (int j = 0; j < g_iterationData.lessonsPerDay; j++)
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
                    int classroomId =
                        classPair.second.days[i].classroomLessonPairs[j].classroomIds[k];
                    teacherClassrooms[lessonTeacherPair.teacherId][i].lessonIds[j] = classroomId;
                }
            }
        }
    }
    return teacherClassrooms;
}

static void GetTeacherCollisionErrors(Timetable& timetable)
{
    std::unordered_map<int, std::vector<Day>> teacherLessons;
    for (auto& teacher: timetable.teachers)
    {
        teacherLessons[teacher.first].resize(g_iterationData.daysPerWeek);
        for (int i = 0; i < g_iterationData.daysPerWeek; i++)
        {
            teacherLessons[teacher.first][i].lessons.resize(g_iterationData.lessonsPerDay, false);
        }
    }
    for (auto& classPair: timetable.classes)
    {
        classPair.second.days.resize(g_iterationData.daysPerWeek);
        for (int i = 0; i < g_iterationData.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(g_iterationData.lessonsPerDay);
            for (int j = 0; j < g_iterationData.lessonsPerDay; j++)
            {
                int timetableLessonId =
                    classPair.second.days[i].classroomLessonPairs[j].timetableLessonId;
                if (timetableLessonId < 0) continue;
                for (auto& lessonTeacherPair:
                     classPair.second.timetableLessons[timetableLessonId].lessonTeacherPairs)
                {
                    int& teacherId = lessonTeacherPair.teacherId;
                    if (!teacherLessons[teacherId][i].lessons[j])
                        teacherLessons[teacherId][i].lessons[j] = true;
                    else
                    {
                        timetable.errors++;
                        if (g_settings.verboseLogging) LogInfo("Teacher collision error");
                    }
                }
            }
        }
    }
}

static void GetClassroomCollisionErrors(Timetable& timetable)
{
    std::unordered_map<int, std::vector<Day>> classroomLessons;
    for (auto& classroom: timetable.classrooms)
    {
        classroomLessons[classroom.first].resize(g_iterationData.daysPerWeek);
        for (int i = 0; i < g_iterationData.daysPerWeek; i++)
        {
            classroomLessons[classroom.first][i].lessons.resize(g_iterationData.lessonsPerDay, false);
        }
    }
    for (auto& classPair: timetable.classes)
    {
        classPair.second.days.resize(g_iterationData.daysPerWeek);
        for (int i = 0; i < g_iterationData.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(g_iterationData.lessonsPerDay);
            for (int j = 0; j < g_iterationData.lessonsPerDay; j++)
            {
                int timetableLessonId =
                    classPair.second.days[i].classroomLessonPairs[j].timetableLessonId;
                if (timetableLessonId < 0) continue;
                for (size_t k = 0;
                     k < classPair.second.days[i].classroomLessonPairs[j].classroomIds.size(); k++)
                {
                    int classroomId =
                        classPair.second.days[i].classroomLessonPairs[j].classroomIds[k];
                    if (!classroomLessons[classroomId][i].lessons[j])
                        classroomLessons[classroomId][i].lessons[j] = true;
                    else
                    {
                        timetable.errors++;
                        if (g_settings.verboseLogging) LogInfo("Classroom collision error");
                    }
                }
            }
        }
    }
}

static void GetLessonCollisionErrors(Timetable& timetable)
{
    for (auto& classPair: timetable.classes)
    {
        std::unordered_map<int, int> totalLessonIntersections;
        for (auto& lesson: classPair.second.timetableLessons)
            totalLessonIntersections[lesson.first] = 0;
        int classDays = g_iterationData.daysPerWeek;
        for (int i = 0; i < g_iterationData.daysPerWeek; i++)
        {
            std::unordered_map<int, int> lessonIntersections;
            for (auto& lesson: classPair.second.timetableLessons)
                lessonIntersections[lesson.first] = 0;
            bool foundAvailableLesson = false;
            classPair.second.days.resize(g_iterationData.daysPerWeek);
            for (int j = 0; j < g_iterationData.lessonsPerDay; j++)
            {
                if (classPair.second.days[i].lessons[j]) foundAvailableLesson = true;
                int timetableLessonId =
                    classPair.second.days[i].classroomLessonPairs[j].timetableLessonId;
                if (timetableLessonId < 0) continue;
                if (++lessonIntersections[timetableLessonId] > 1)
                    totalLessonIntersections[timetableLessonId]++;
            }
            if (!foundAvailableLesson) classDays--;
        }
        for (auto& lesson: classPair.second.timetableLessons)
        {
            if (totalLessonIntersections[lesson.first] >
                std::max(0, lesson.second.count - classDays))
            {
                timetable.errors +=
                    totalLessonIntersections[lesson.first] - (lesson.second.count - classDays);
                if (g_settings.verboseLogging) LogInfo("Lesson collision error");
            }
        }
    }
}

static void GetTemplateMatchErrors(Timetable& timetable,
                                   std::unordered_map<int, std::vector<WorkDay>> teacherLessons)
{
    for (auto& teacher: timetable.teachers)
    {
        teacher.second.workDays.resize(g_iterationData.daysPerWeek);
        for (int i = 0; i < g_iterationData.daysPerWeek; i++)
        {
            teacher.second.workDays[i].lessonIds.resize(g_iterationData.lessonsPerDay);
            for (int j = 0; j < g_iterationData.lessonsPerDay; j++)
            {
                int teacherLesson = teacher.second.workDays[i].lessonIds[j];
                int classLesson = teacherLessons[teacher.first][i].lessonIds[j];
                if (teacherLesson == ANY_LESSON || teacherLesson == -1 ||
                    classLesson == ANY_LESSON || classLesson == -1)
                    continue;

                if (teacherLesson != classLesson)
                {
                    timetable.errors++;
                    if (g_settings.verboseLogging) LogInfo("Template match error");
                }
            }
        }
    }
}

static void GetFreePeriodErrors(Timetable& timetable,
                                std::unordered_map<int, std::vector<WorkDay>> teacherLessons)
{
    std::unordered_map<int, int> teacherFreePeriods;
    for (auto& teacher: timetable.teachers)
    {
        teacherFreePeriods[teacher.first] = 0;
        for (int i = 0; i < g_iterationData.daysPerWeek; i++)
        {
            int firstLessonIndex = -1;
            int lastLessonIndex = -1;
            for (int j = 0; j < g_iterationData.lessonsPerDay; j++)
            {
                if (teacherLessons[teacher.first][i].lessonIds[j] >= 0)
                {
                    firstLessonIndex = j;
                    break;
                }
            }
            for (int j = g_iterationData.lessonsPerDay - 1; j >= 0; j--)
            {
                if (teacherLessons[teacher.first][i].lessonIds[j] >= 0)
                {
                    lastLessonIndex = j;
                    break;
                }
            }
            if (firstLessonIndex == -1 || lastLessonIndex == -1) continue;
            for (int j = firstLessonIndex; j <= lastLessonIndex; j++)
            {
                int& teacherLesson = teacherLessons[teacher.first][i].lessonIds[j];
                if (teacherLesson < 0) teacherFreePeriods[teacher.first]++;
            }
        }
        if (teacherFreePeriods[teacher.first] < g_settings.minFreePeriods)
        {
            timetable.errors += g_settings.minFreePeriods - teacherFreePeriods[teacher.first];
            if (g_settings.verboseLogging) LogInfo("Too little teacher free periods error");
        }
        if (teacherFreePeriods[teacher.first] > g_settings.maxFreePeriods)
        {
            timetable.errors += teacherFreePeriods[teacher.first] - g_settings.maxFreePeriods;
            if (g_settings.verboseLogging) LogInfo("Too many teacher free periods error");
        }
    }
}

static void GetLessonGapErrors(Timetable& timetable)
{
    for (auto& classPair: timetable.classes)
    {
        classPair.second.days.resize(g_iterationData.daysPerWeek);
        for (int i = 0; i < g_iterationData.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(g_iterationData.lessonsPerDay);
            int firstLessonIndex = -1;
            int lastLessonIndex = -1;
            for (int j = 0; j < g_iterationData.lessonsPerDay; j++)
            {
                if (classPair.second.days[i].classroomLessonPairs[j].timetableLessonId >= 0)
                {
                    firstLessonIndex = j;
                    break;
                }
            }
            for (int j = g_iterationData.lessonsPerDay - 1; j >= 0; j--)
            {
                if (classPair.second.days[i].classroomLessonPairs[j].timetableLessonId >= 0)
                {
                    lastLessonIndex = j;
                    break;
                }
            }
            if (firstLessonIndex == -1 || lastLessonIndex == -1) continue;
            for (int j = firstLessonIndex; j <= lastLessonIndex; j++)
            {
                int& timetableLessonId =
                    classPair.second.days[i].classroomLessonPairs[j].timetableLessonId;
                if (timetableLessonId < 0)
                {
                    timetable.errors++;
                    if (g_settings.verboseLogging) LogInfo("Lesson gap error");
                }
            }
        }
    }
}

std::vector<TimetableLessonRule> GetAllRuleVariants(const TimetableLessonRule& timetableLessonRule)
{
    std::vector<TimetableLessonRule> ruleVariants;
    if (timetableLessonRule.preserveOrder)
    {
        ruleVariants.push_back(TimetableLessonRule());
        ruleVariants[0].timetableLessonIds = timetableLessonRule.timetableLessonIds;
        ruleVariants[0].count = timetableLessonRule.count;
        return ruleVariants;
    }
    std::vector<int> timetableLessonIds = timetableLessonRule.timetableLessonIds;
    std::set<std::vector<int>> idSet;
    std::sort(timetableLessonIds.begin(), timetableLessonIds.end());
    size_t index = 0;
    do
    {
        idSet.insert(timetableLessonIds);
    } while (std::next_permutation(timetableLessonIds.begin(), timetableLessonIds.end()));
    for (const auto& ids: idSet)
    {
        ruleVariants.push_back(TimetableLessonRule());
        ruleVariants[index].timetableLessonIds = ids;
        ruleVariants[index].count = timetableLessonRule.count;
        index++;
    }
    return ruleVariants;
}

static int GetRuleCount(Class classPair, const std::vector<int>& timetableLessonIds)
{
    int output = 0;
    classPair.days.resize(g_iterationData.daysPerWeek);
    for (int i = 0; i < g_iterationData.daysPerWeek; i++)
    {
        std::vector<int> classTimetableLessonIds;
        for (auto& classroomLessonPair: classPair.days[i].classroomLessonPairs)
        {
            classTimetableLessonIds.push_back(classroomLessonPair.timetableLessonId);
        }
        for (size_t j = 0; j + timetableLessonIds.size() < classTimetableLessonIds.size(); j++)
        {
            bool match = true;
            for (size_t k = 0; k < timetableLessonIds.size(); k++)
            {
                if (j + k >= classTimetableLessonIds.size() || k >= timetableLessonIds.size() ||
                    classTimetableLessonIds[j + k] != timetableLessonIds[k])
                {
                    match = false;
                    break;
                }
            }
            if (match)
            {
                output++;
            }
        }
    }
    return output;
}

static void GetTimetableLessonRulesErrors(Timetable& timetable)
{
    for (auto& classPair: timetable.classes)
    {
        for (size_t i = 0; i < g_iterationData.classRuleVariants[classPair.first].size(); i++)
        {
            int totalRuleCount = 0;
            std::vector<TimetableLessonRule>& ruleVariants =
                g_iterationData.classRuleVariants[classPair.first][i];
            for (size_t j = 0; j < g_iterationData.classRuleVariants[classPair.first][i].size(); j++)
            {
                totalRuleCount +=
                    GetRuleCount(classPair.second, ruleVariants[j].timetableLessonIds);
            }
            if (totalRuleCount < ruleVariants[0].count || totalRuleCount > ruleVariants[0].count)
            {
                timetable.errors++;
                if (g_settings.verboseLogging) LogInfo("Class rule error");
            }
        }
    }
}

static void GetTimetableErrors(Timetable& timetable,
                               std::unordered_map<int, std::vector<WorkDay>> teacherLessons)
{
    // Reset timetable errors
    timetable.errors = 0;

    // Get the same teacher in different classrooms at the same time errors
    GetTeacherCollisionErrors(timetable);

    // Get the same classroom being used at the same time errors
    GetClassroomCollisionErrors(timetable);

    // Get the same timetable lesson in a day errors
    GetLessonCollisionErrors(timetable);

    // Get the errors caused by mismatches between teacher selected timetables and classes timetable
    GetTemplateMatchErrors(timetable, teacherLessons);

    // Get free period out of bounds errors
    GetFreePeriodErrors(timetable, teacherLessons);

    // Get gaps in the timetable errors
    GetLessonGapErrors(timetable);

    // Get timetable lesson rules errors
    GetTimetableLessonRulesErrors(timetable);
}

static void GetTeacherMovementBonusPoints(Timetable& timetable)
{
    std::unordered_map<int, std::vector<WorkDay>> teacherClassrooms =
        GetTeacherClassrooms(timetable);
    for (auto& teacher: timetable.teachers)
    {
        teacherClassrooms[teacher.first].resize(g_iterationData.daysPerWeek);
        for (int i = 0; i < g_iterationData.daysPerWeek; i++)
        {
            teacherClassrooms[teacher.first][i].lessonIds.resize(g_iterationData.lessonsPerDay);
            int lastClassroom = timetable.maxLessonId + 1;
            int firstClassroomIndex = -1;
            int lastClassroomIndex = -1;
            for (int j = 0; j < g_iterationData.lessonsPerDay; j++)
            {
                if (teacherClassrooms[teacher.first][i].lessonIds[j] != NO_LESSON)
                {
                    firstClassroomIndex = j;
                    break;
                }
            }
            for (int j = g_iterationData.lessonsPerDay - 1; j >= 0; j--)
            {
                if (teacherClassrooms[teacher.first][i].lessonIds[j] != NO_LESSON)
                {
                    lastClassroomIndex = j;
                    break;
                }
            }
            if (firstClassroomIndex == -1 || lastClassroomIndex == -1) continue;
            for (int j = firstClassroomIndex; j <= lastClassroomIndex; j++)
            {
                int& teacherClassroom = teacherClassrooms[teacher.first][i].lessonIds[j];
                if (teacherClassroom == lastClassroom) timetable.bonusPoints += 2;
                lastClassroom = teacherClassroom;
            }
        }
    }
}

static void GetStudentMovementBonusPoints(Timetable& timetable)
{
    for (auto& classPair: timetable.classes)
    {
        for (int i = 0; i < g_iterationData.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(g_iterationData.lessonsPerDay);
            int lastClassroom = timetable.maxClassroomId + 1;
            for (int j = 0; j < g_iterationData.lessonsPerDay; j++)
            {
                int timetableLessonId =
                    classPair.second.days[i].classroomLessonPairs[j].timetableLessonId;
                if (timetableLessonId < 0) continue;
                for (size_t k = 0;
                     k <
                     classPair.second.timetableLessons[timetableLessonId].lessonTeacherPairs.size();
                     k++)
                {
                    int classroomId =
                        classPair.second.days[i].classroomLessonPairs[j].classroomIds[k];
                    if (lastClassroom == classroomId) timetable.bonusPoints++;
                    lastClassroom = classroomId;
                }
            }
        }
    }
}

static void GetTimetableBonusPoints(Timetable& timetable)
{
    // Reset timetable bonus points
    timetable.bonusPoints = 0;

    // Get minimal teacher movement bonus points
    GetTeacherMovementBonusPoints(timetable);

    // Get minimal student movement bonus points
    GetStudentMovementBonusPoints(timetable);
}

void ScoreTimetable(Timetable& timetable)
{
    // Pre-calculate teacher lessons
    std::unordered_map<int, std::vector<WorkDay>> teacherLessons = GetTeacherLessons(timetable);

    GetTimetableErrors(timetable, teacherLessons);
    GetTimetableBonusPoints(timetable);
}

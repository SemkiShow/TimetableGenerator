// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Searching.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"
#include <algorithm>
#include <iostream>
#include <set>
#include <unordered_map>
#include <vector>

std::unordered_map<int, std::vector<WorkDay>> GetTeacherLessons(Timetable& timetable)
{
    std::unordered_map<int, std::vector<WorkDay>> teacherLessons;
    for (auto& teacher: timetable.teachers)
    {
        teacherLessons[teacher.first].resize(iterationData.daysPerWeek);
        for (size_t i = 0; i < iterationData.daysPerWeek; i++)
        {
            teacherLessons[teacher.first][i].lessonIds.resize(iterationData.lessonsPerDay,
                                                              NO_LESSON);
        }
    }
    for (auto& classPair: timetable.classes)
    {
        classPair.second.days.resize(iterationData.daysPerWeek);
        for (size_t i = 0; i < iterationData.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(iterationData.lessonsPerDay);
            for (size_t j = 0; j < iterationData.lessonsPerDay; j++)
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
std::unordered_map<int, std::vector<WorkDay>> GetTeacherClassrooms(Timetable& timetable)
{
    std::unordered_map<int, std::vector<WorkDay>> teacherClassrooms;
    for (auto& teacher: timetable.teachers)
    {
        teacherClassrooms[teacher.first].resize(iterationData.daysPerWeek);
        for (size_t i = 0; i < iterationData.daysPerWeek; i++)
        {
            teacherClassrooms[teacher.first][i].lessonIds.resize(iterationData.lessonsPerDay,
                                                                 NO_LESSON);
        }
    }
    for (auto& classPair: timetable.classes)
    {
        classPair.second.days.resize(iterationData.daysPerWeek);
        for (size_t i = 0; i < iterationData.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(iterationData.lessonsPerDay);
            for (size_t j = 0; j < iterationData.lessonsPerDay; j++)
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

void GetTeacherCollisionErrors(Timetable& timetable)
{
    std::unordered_map<int, std::vector<Day>> teacherLessons;
    for (auto& teacher: timetable.teachers)
    {
        teacherLessons[teacher.first].resize(iterationData.daysPerWeek);
        for (size_t i = 0; i < iterationData.daysPerWeek; i++)
        {
            teacherLessons[teacher.first][i].lessons.resize(iterationData.lessonsPerDay, false);
        }
    }
    for (auto& classPair: timetable.classes)
    {
        classPair.second.days.resize(iterationData.daysPerWeek);
        for (size_t i = 0; i < iterationData.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(iterationData.lessonsPerDay);
            for (size_t j = 0; j < iterationData.lessonsPerDay; j++)
            {
                int timetableLessonId =
                    classPair.second.days[i].classroomLessonPairs[j].timetableLessonId;
                if (timetableLessonId < 0) continue;
                for (size_t k = 0;
                     k <
                     classPair.second.timetableLessons[timetableLessonId].lessonTeacherPairs.size();
                     k++)
                {
                    int& teacherId = classPair.second.timetableLessons[timetableLessonId]
                                         .lessonTeacherPairs[k]
                                         .teacherId;
                    if (!teacherLessons[teacherId][i].lessons[j])
                        teacherLessons[teacherId][i].lessons[j] = true;
                    else
                    {
                        timetable.errors++;
                        if (settings.verboseLogging)
                        {
                            std::cout << "Teacher collision error. ";
                        }
                    }
                }
            }
        }
    }
}

void GetClassroomCollisionErrors(Timetable& timetable)
{
    std::unordered_map<int, std::vector<Day>> classroomLessons;
    for (auto& classroom: timetable.classrooms)
    {
        classroomLessons[classroom.first].resize(iterationData.daysPerWeek);
        for (size_t i = 0; i < iterationData.daysPerWeek; i++)
        {
            classroomLessons[classroom.first][i].lessons.resize(iterationData.lessonsPerDay, false);
        }
    }
    for (auto& classPair: timetable.classes)
    {
        classPair.second.days.resize(iterationData.daysPerWeek);
        for (size_t i = 0; i < iterationData.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(iterationData.lessonsPerDay);
            for (size_t j = 0; j < iterationData.lessonsPerDay; j++)
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
                        if (settings.verboseLogging)
                        {
                            std::cout << "Classroom collision error. ";
                        }
                    }
                }
            }
        }
    }
}

void GetLessonCollisionErrors(Timetable& timetable)
{
    for (auto& classPair: timetable.classes)
    {
        std::unordered_map<int, int> totalLessonIntersections;
        for (auto& lesson: classPair.second.timetableLessons)
            totalLessonIntersections[lesson.first] = 0;
        int classDays = iterationData.daysPerWeek;
        for (size_t i = 0; i < iterationData.daysPerWeek; i++)
        {
            std::unordered_map<int, int> lessonIntersections;
            for (auto& lesson: classPair.second.timetableLessons)
                lessonIntersections[lesson.first] = 0;
            bool foundAvailableLesson = false;
            classPair.second.days.resize(iterationData.daysPerWeek);
            for (size_t j = 0; j < iterationData.lessonsPerDay; j++)
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
                std::max(0, lesson.second.amount - classDays))
            {
                timetable.errors +=
                    totalLessonIntersections[lesson.first] - (lesson.second.amount - classDays);
                if (settings.verboseLogging)
                {
                    std::cout << "Lesson collision error. ";
                }
            }
        }
    }
}

void GetTemplateMatchErrors(Timetable& timetable,
                            std::unordered_map<int, std::vector<WorkDay>> teacherLessons)
{
    for (auto& teacher: timetable.teachers)
    {
        teacher.second.workDays.resize(iterationData.daysPerWeek);
        for (size_t i = 0; i < iterationData.daysPerWeek; i++)
        {
            teacher.second.workDays[i].lessonIds.resize(iterationData.lessonsPerDay);
            for (size_t j = 0; j < iterationData.lessonsPerDay; j++)
            {
                int teacherLesson = teacher.second.workDays[i].lessonIds[j];
                int classLesson = teacherLessons[teacher.first][i].lessonIds[j];
                if (teacherLesson == ANY_LESSON || teacherLesson == -1 ||
                    classLesson == ANY_LESSON || classLesson == -1)
                    continue;
                else if (teacherLesson != classLesson)
                {
                    timetable.errors++;
                    if (settings.verboseLogging)
                    {
                        std::cout << "Template match error. ";
                    }
                }
            }
        }
    }
}

void GetFreePeriodErrors(Timetable& timetable,
                         std::unordered_map<int, std::vector<WorkDay>> teacherLessons)
{
    std::unordered_map<int, int> teacherFreePeriods;
    for (auto& teacher: timetable.teachers)
    {
        teacherFreePeriods[teacher.first] = 0;
        for (size_t i = 0; i < iterationData.daysPerWeek; i++)
        {
            int firstLessonIndex = -1;
            int lastLessonIndex = -1;
            for (size_t j = 0; j < iterationData.lessonsPerDay; j++)
            {
                if (teacherLessons[teacher.first][i].lessonIds[j] >= 0)
                {
                    firstLessonIndex = j;
                    break;
                }
            }
            for (int j = iterationData.lessonsPerDay - 1; j >= 0; j--)
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
        if (teacherFreePeriods[teacher.first] < settings.minFreePeriods)
        {
            timetable.errors += settings.minFreePeriods - teacherFreePeriods[teacher.first];
            if (settings.verboseLogging)
            {
                std::cout << "Too little teacher free periods error. ";
            }
        }
        if (teacherFreePeriods[teacher.first] > settings.maxFreePeriods)
        {
            timetable.errors += teacherFreePeriods[teacher.first] - settings.maxFreePeriods;
            if (settings.verboseLogging)
            {
                std::cout << "Too many teacher free periods error. ";
            }
        }
    }
}

void GetLessonGapErrors(Timetable& timetable)
{
    for (auto& classPair: timetable.classes)
    {
        classPair.second.days.resize(iterationData.daysPerWeek);
        for (size_t i = 0; i < iterationData.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(iterationData.lessonsPerDay);
            int firstLessonIndex = -1;
            int lastLessonIndex = -1;
            for (size_t j = 0; j < iterationData.lessonsPerDay; j++)
            {
                if (classPair.second.days[i].classroomLessonPairs[j].timetableLessonId >= 0)
                {
                    firstLessonIndex = j;
                    break;
                }
            }
            for (int j = iterationData.lessonsPerDay - 1; j >= 0; j--)
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
                    if (settings.verboseLogging)
                    {
                        std::cout << "Lesson gap error. ";
                    }
                }
            }
        }
    }
}

std::vector<TimetableLessonRule> GetAllRuleVariants(const TimetableLessonRule timetableLessonRule)
{
    std::vector<TimetableLessonRule> ruleVariants;
    if (timetableLessonRule.preserveOrder)
    {
        ruleVariants.push_back(TimetableLessonRule());
        ruleVariants[0].timetableLessonIds = timetableLessonRule.timetableLessonIds;
        ruleVariants[0].amount = timetableLessonRule.amount;
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
        ruleVariants[index].amount = timetableLessonRule.amount;
        index++;
    }
    return ruleVariants;
}

std::vector<int> GetVectorSlice(const std::vector<int> input, size_t begin, size_t end)
{
    if (begin >= input.size() || end >= input.size()) return input;
    std::vector<int> slice(input.begin() + begin, input.begin() + end);
    return slice;
}

int GetRuleAmount(Class classPair, const std::vector<int> timetableLessonIds)
{
    int output = 0;
    classPair.days.resize(iterationData.daysPerWeek);
    for (size_t i = 0; i < iterationData.daysPerWeek; i++)
    {
        std::vector<int> classTimetableLessonIds;
        for (size_t j = 0; j < classPair.days[i].classroomLessonPairs.size(); j++)
        {
            classTimetableLessonIds.push_back(
                classPair.days[i].classroomLessonPairs[j].timetableLessonId);
        }
        for (size_t j = 0; j + timetableLessonIds.size() < classTimetableLessonIds.size(); j++)
        {
            if (GetVectorSlice(classTimetableLessonIds, j, j + timetableLessonIds.size()) ==
                timetableLessonIds)
            {
                output++;
            }
        }
    }
    return output;
}

void GetTimetableLessonRulesErrors(Timetable& timetable)
{
    for (auto& classPair: timetable.classes)
    {
        for (size_t i = 0; i < iterationData.classRuleVariants[classPair.first].size(); i++)
        {
            int totalRuleAmount = 0;
            std::vector<TimetableLessonRule>& ruleVariants =
                iterationData.classRuleVariants[classPair.first][i];
            for (size_t j = 0; j < iterationData.classRuleVariants[classPair.first][i].size(); j++)
            {
                totalRuleAmount +=
                    GetRuleAmount(classPair.second, ruleVariants[j].timetableLessonIds);
            }
            if (totalRuleAmount < ruleVariants[0].amount ||
                totalRuleAmount > ruleVariants[0].amount)
            {
                timetable.errors++;
                if (settings.verboseLogging)
                {
                    std::cout << "Class rule error. ";
                }
            }
        }
    }
}

void GetTimetableErrors(Timetable& timetable,
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

void GetTeacherMovementBonusPoints(Timetable& timetable)
{
    std::unordered_map<int, std::vector<WorkDay>> teacherClassrooms =
        GetTeacherClassrooms(timetable);
    for (auto& teacher: timetable.teachers)
    {
        teacherClassrooms[teacher.first].resize(iterationData.daysPerWeek);
        for (size_t i = 0; i < iterationData.daysPerWeek; i++)
        {
            teacherClassrooms[teacher.first][i].lessonIds.resize(iterationData.lessonsPerDay);
            int lastClassroom = timetable.maxLessonId + 1;
            int firstClassroomIndex = -1;
            int lastClassroomIndex = -1;
            for (size_t j = 0; j < iterationData.lessonsPerDay; j++)
            {
                if (teacherClassrooms[teacher.first][i].lessonIds[j] != NO_LESSON)
                {
                    firstClassroomIndex = j;
                    break;
                }
            }
            for (int j = iterationData.lessonsPerDay - 1; j >= 0; j--)
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

void GetStudentMovementBonusPoints(Timetable& timetable)
{
    for (auto& classPair: timetable.classes)
    {
        for (size_t i = 0; i < iterationData.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(iterationData.lessonsPerDay);
            int lastClassroom = timetable.maxClassroomId + 1;
            for (size_t j = 0; j < iterationData.lessonsPerDay; j++)
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

void GetTimetableBonusPoints(Timetable& timetable)
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

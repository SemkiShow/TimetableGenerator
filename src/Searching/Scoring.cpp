#include "Searching.hpp"
#include "Settings.hpp"
#include <iostream>
#include <unordered_map>

std::unordered_map<int, std::vector<WorkDay>> GetTeacherLessons(Timetable* timetable)
{
    std::unordered_map<int, std::vector<WorkDay>> teacherLessons;
    for (auto& teacher: timetable->teachers)
    {
        teacherLessons[teacher.first].resize(iterationData.daysPerWeek);
        for (int i = 0; i < iterationData.daysPerWeek; i++)
        {
            teacherLessons[teacher.first][i].lessonIDs.resize(iterationData.lessonsPerDay, NO_LESSON);
        }
    }
    for (auto& classPair: timetable->classes)
    {
        classPair.second.days.resize(iterationData.daysPerWeek);
        for (int i = 0; i < iterationData.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(iterationData.lessonsPerDay);
            for (int j = 0; j < iterationData.lessonsPerDay; j++)
            {
                int timetableLessonID = classPair.second.days[i].classroomLessonPairs[j].timetableLessonID;
                if (timetableLessonID < 0) continue;
                for (int k = 0; k < classPair.second.timetableLessons[timetableLessonID].lessonTeacherPairs.size(); k++)
                {
                    LessonTeacherPair& lessonTeacherPair = classPair.second.timetableLessons[timetableLessonID].lessonTeacherPairs[k];
                    teacherLessons[lessonTeacherPair.teacherID][i].lessonIDs[j] = lessonTeacherPair.lessonID;
                }
            }
        }
    }
    return teacherLessons;
}

// I use lessonIDs here in place of non-existent classroomIDs
std::unordered_map<int, std::vector<WorkDay>> GetTeacherClassrooms(Timetable* timetable)
{
    std::unordered_map<int, std::vector<WorkDay>> teacherClassrooms;
    for (auto& teacher: timetable->teachers)
    {
        teacherClassrooms[teacher.first].resize(iterationData.daysPerWeek);
        for (int i = 0; i < iterationData.daysPerWeek; i++)
        {
            teacherClassrooms[teacher.first][i].lessonIDs.resize(iterationData.lessonsPerDay, NO_LESSON);
        }
    }
    for (auto& classPair: timetable->classes)
    {
        classPair.second.days.resize(iterationData.daysPerWeek);
        for (int i = 0; i < iterationData.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(iterationData.lessonsPerDay);
            for (int j = 0; j < iterationData.lessonsPerDay; j++)
            {
                int timetableLessonID = classPair.second.days[i].classroomLessonPairs[j].timetableLessonID;
                if (timetableLessonID < 0) continue;
                for (int k = 0; k < classPair.second.timetableLessons[timetableLessonID].lessonTeacherPairs.size(); k++)
                {
                    LessonTeacherPair& lessonTeacherPair = classPair.second.timetableLessons[timetableLessonID].lessonTeacherPairs[k];
                    int classroomID = classPair.second.days[i].classroomLessonPairs[j].classroomIDs[k];
                    teacherClassrooms[lessonTeacherPair.teacherID][i].lessonIDs[j] = classroomID;
                }
            }
        }
    }
    return teacherClassrooms;
}

void GetTeacherCollisionErrors(Timetable* timetable)
{
    std::unordered_map<int, std::vector<Day>> teacherLessons;
    for (auto& teacher: timetable->teachers)
    {
        teacherLessons[teacher.first].resize(iterationData.daysPerWeek);
        for (int i = 0; i < iterationData.daysPerWeek; i++)
        {
            teacherLessons[teacher.first][i].lessons.resize(iterationData.lessonsPerDay, false);
        }
    }
    for (auto& classPair: timetable->classes)
    {
        classPair.second.days.resize(iterationData.daysPerWeek);
        for (int i = 0; i < iterationData.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(iterationData.lessonsPerDay);
            for (int j = 0; j < iterationData.lessonsPerDay; j++)
            {
                int timetableLessonID = classPair.second.days[i].classroomLessonPairs[j].timetableLessonID;
                if (timetableLessonID < 0) continue;
                for (int k = 0; k < classPair.second.timetableLessons[timetableLessonID].lessonTeacherPairs.size(); k++)
                {
                    int& teacherID = classPair.second.timetableLessons[timetableLessonID].lessonTeacherPairs[k].teacherID;
                    if (!teacherLessons[teacherID][i].lessons[j])
                        teacherLessons[teacherID][i].lessons[j] = true;
                    else
                    {
                        timetable->errors++;
                        if (verboseLogging)
                        {
                            std::cout << "Teacher collision error. ";
                        }
                    }
                }
            }
        }
    }
}

void GetClassroomCollisionErrors(Timetable* timetable)
{
    std::unordered_map<int, std::vector<Day>> classroomLessons;
    for (auto& classroom: timetable->classrooms)
    {
        classroomLessons[classroom.first].resize(iterationData.daysPerWeek);
        for (int i = 0; i < iterationData.daysPerWeek; i++)
        {
            classroomLessons[classroom.first][i].lessons.resize(iterationData.lessonsPerDay, false);
        }
    }
    for (auto& classPair: timetable->classes)
    {
        classPair.second.days.resize(iterationData.daysPerWeek);
        for (int i = 0; i < iterationData.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(iterationData.lessonsPerDay);
            for (int j = 0; j < iterationData.lessonsPerDay; j++)
            {
                int timetableLessonID = classPair.second.days[i].classroomLessonPairs[j].timetableLessonID;
                if (timetableLessonID < 0) continue;
                for (int k = 0; k < classPair.second.days[i].classroomLessonPairs[j].classroomIDs.size(); k++)
                {
                    int classroomID = classPair.second.days[i].classroomLessonPairs[j].classroomIDs[k];
                    if (!classroomLessons[classroomID][i].lessons[j])
                        classroomLessons[classroomID][i].lessons[j] = true;
                    else
                    {
                        timetable->errors++;
                        if (verboseLogging)
                        {
                            std::cout << "Classroom collision error. ";
                        }
                    }
                }
            }
        }
    }
}

void GetLessonCollisionErrors(Timetable* timetable)
{
    for (auto& classPair: timetable->classes)
    {
        std::unordered_map<int, int> totalLessonIntersections;
        for (auto& lesson: classPair.second.timetableLessons)
            totalLessonIntersections[lesson.first] = 0;
        int classDays = iterationData.daysPerWeek;
        for (int i = 0; i < iterationData.daysPerWeek; i++)
        {
            std::unordered_map<int, int> lessonIntersections;
            for (auto& lesson: classPair.second.timetableLessons)
                lessonIntersections[lesson.first] = 0;
            bool foundAvailableLesson = false;
            classPair.second.days.resize(iterationData.daysPerWeek);
            for (int j = 0; j < iterationData.lessonsPerDay; j++)
            {
                if (classPair.second.days[i].lessons[j]) foundAvailableLesson = true;
                int timetableLessonID = classPair.second.days[i].classroomLessonPairs[j].timetableLessonID;
                if (timetableLessonID < 0) continue;
                if (++lessonIntersections[timetableLessonID] > 1)
                    totalLessonIntersections[timetableLessonID]++;
            }
            if (!foundAvailableLesson) classDays--;
        }
        for (auto& lesson: classPair.second.timetableLessons)
        {
            if (totalLessonIntersections[lesson.first] > lesson.second.amount - classDays)
            {
                timetable->errors += totalLessonIntersections[lesson.first] - (lesson.second.amount - classDays);
                if (verboseLogging)
                {
                    std::cout << "Lesson collision error. ";
                }
            }
        }
    }
}

void GetTemplateMatchErrors(Timetable* timetable, std::unordered_map<int, std::vector<WorkDay>> teacherLessons)
{
    for (auto& teacher: timetable->teachers)
    {
        teacher.second.workDays.resize(iterationData.daysPerWeek);
        for (int i = 0; i < iterationData.daysPerWeek; i++)
        {
            teacher.second.workDays[i].lessonIDs.resize(iterationData.lessonsPerDay);
            for (int j = 0; j < iterationData.lessonsPerDay; j++)
            {
                int teacherLesson = teacher.second.workDays[i].lessonIDs[j];
                int classLesson = teacherLessons[teacher.first][i].lessonIDs[j];
                if (teacherLesson == ANY_LESSON || teacherLesson == -1 || classLesson == ANY_LESSON || classLesson == -1) continue;
                else if (teacherLesson != classLesson)
                {
                    timetable->errors++;
                    if (verboseLogging)
                    {
                        std::cout << "Template match error. ";
                    }
                }
            }
        }
    }
}

void GetFreePeriodErrors(Timetable* timetable, std::unordered_map<int, std::vector<WorkDay>> teacherLessons)
{
    std::unordered_map<int, int> teacherFreePeriods;
    for (auto& teacher: timetable->teachers)
    {
        teacherFreePeriods[teacher.first] = 0;
        for (int i = 0; i < iterationData.daysPerWeek; i++)
        {
            int firstLessonIndex = -1;
            int lastLessonIndex = -1;
            for (int j = 0; j < iterationData.lessonsPerDay; j++)
            {
                if (teacherLessons[teacher.first][i].lessonIDs[j] >= 0)
                {
                    firstLessonIndex = j;
                    break;
                }
            }
            for (int j = iterationData.lessonsPerDay-1; j >= 0; j--)
            {
                if (teacherLessons[teacher.first][i].lessonIDs[j] >= 0)
                {
                    lastLessonIndex = j;
                    break;
                }
            }
            if (firstLessonIndex == -1 || lastLessonIndex == -1) continue;
            for (int j = firstLessonIndex; j <= lastLessonIndex; j++)
            {
                int& teacherLesson = teacherLessons[teacher.first][i].lessonIDs[j];
                if (teacherLesson < 0) teacherFreePeriods[teacher.first]++;
            }
        }
        if (teacherFreePeriods[teacher.first] < minFreePeriods)
        {
            timetable->errors += minFreePeriods - teacherFreePeriods[teacher.first];
            if (verboseLogging)
            {
                std::cout << teacher.second.name << ". " << teacherFreePeriods[teacher.first] << ". ";
                std::cout << "Too little teacher free periods error. ";
            }
        }
        if (teacherFreePeriods[teacher.first] > maxFreePeriods)
        {
            timetable->errors += teacherFreePeriods[teacher.first] - maxFreePeriods;
            if (verboseLogging)
            {
                std::cout << teacher.second.name << ". " << teacherFreePeriods[teacher.first] << ". ";
                std::cout << "Too many teacher free periods error. ";
            }
        }
    }
}

void GetLessonGapErrors(Timetable* timetable)
{
    for (auto& classPair: timetable->classes)
    {
        classPair.second.days.resize(iterationData.daysPerWeek);
        for (int i = 0; i < iterationData.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(iterationData.lessonsPerDay);
            int firstLessonIndex = -1;
            int lastLessonIndex = -1;
            for (int j = 0; j < iterationData.lessonsPerDay; j++)
            {
                if (classPair.second.days[i].classroomLessonPairs[j].timetableLessonID >= 0)
                {
                    firstLessonIndex = j;
                    break;
                }
            }
            for (int j = iterationData.lessonsPerDay-1; j >= 0; j--)
            {
                if (classPair.second.days[i].classroomLessonPairs[j].timetableLessonID >= 0)
                {
                    lastLessonIndex = j;
                    break;
                }
            }
            if (firstLessonIndex == -1 || lastLessonIndex == -1) continue;
            for (int j = firstLessonIndex; j <= lastLessonIndex; j++)
            {
                int& timetableLessonID = classPair.second.days[i].classroomLessonPairs[j].timetableLessonID;
                if (timetableLessonID < 0)
                {
                    timetable->errors++;
                    if (verboseLogging)
                    {
                        std::cout << "Lesson gap error. ";
                    }
                }
            }
        }
    }
}

void GetTimetableErrors(Timetable* timetable, std::unordered_map<int, std::vector<WorkDay>> teacherLessons)
{
    // Reset timetable errors
    timetable->errors = 0;

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
}

void GetTeacherMovementBonusPoints(Timetable* timetable)
{
    std::unordered_map<int, std::vector<WorkDay>> teacherClassrooms = GetTeacherClassrooms(timetable);
    for (auto& teacher: timetable->teachers)
    {
        teacherClassrooms[teacher.first].resize(iterationData.daysPerWeek);
        for (int i = 0; i < iterationData.daysPerWeek; i++)
        {
            teacherClassrooms[teacher.first][i].lessonIDs.resize(iterationData.lessonsPerDay);
            int lastClassroom = timetable->maxLessonID+1;
            int firstClassroomIndex = -1;
            int lastClassroomIndex = -1;
            for (int j = 0; j < iterationData.lessonsPerDay; j++)
            {
                if (teacherClassrooms[teacher.first][i].lessonIDs[j] != NO_LESSON)
                {
                    firstClassroomIndex = j;
                    break;
                }
            }
            for (int j = iterationData.lessonsPerDay-1; j >= 0; j--)
            {
                if (teacherClassrooms[teacher.first][i].lessonIDs[j] != NO_LESSON)
                {
                    lastClassroomIndex = j;
                    break;
                }
            }
            if (firstClassroomIndex == -1 || lastClassroomIndex == -1) continue;
            for (int j = firstClassroomIndex; j <= lastClassroomIndex; j++)
            {
                int& teacherClassroom = teacherClassrooms[teacher.first][i].lessonIDs[j];
                if (teacherClassroom == lastClassroom) timetable->bonusPoints += 2;
                lastClassroom = teacherClassroom;
            }
        }
    }
}

void GetStudentMovementBonusPoints(Timetable* timetable)
{
    for (auto& classPair: timetable->classes)
    {
        for (int i = 0; i < iterationData.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.resize(iterationData.lessonsPerDay);
            int lastClassroom = timetable->maxClassroomID+1;
            for (int j = 0; j < iterationData.lessonsPerDay; j++)
            {
                int timetableLessonID = classPair.second.days[i].classroomLessonPairs[j].timetableLessonID;
                if (timetableLessonID < 0) continue;
                for (int k = 0; k < classPair.second.timetableLessons[timetableLessonID].lessonTeacherPairs.size(); k++)
                {
                    int classroomID = classPair.second.days[i].classroomLessonPairs[j].classroomIDs[k];
                    if (lastClassroom == classroomID) timetable->bonusPoints++;
                    lastClassroom = classroomID;
                }
            }
        }
    }
}

void GetTimetableBonusPoints(Timetable* timetable)
{
    // Reset timetable bonus points
    timetable->bonusPoints = 0;

    // Get minimal teacher movement bonus points
    GetTeacherMovementBonusPoints(timetable);

    // Get minimal student movement bonus points
    GetStudentMovementBonusPoints(timetable);
}

void ScoreTimetable(Timetable* timetable)
{
    // Pre-calculate teacher lessons
    std::unordered_map<int, std::vector<WorkDay>> teacherLessons = GetTeacherLessons(timetable);

    GetTimetableErrors(timetable, teacherLessons);
    GetTimetableBonusPoints(timetable);
}

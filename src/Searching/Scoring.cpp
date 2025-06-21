#include "Searching.hpp"
#include "Settings.hpp"

std::unordered_map<int, WorkDay[DAYS_PER_WEEK]> GetTeacherLessons(Timetable* timetable)
{
    std::unordered_map<int, WorkDay[DAYS_PER_WEEK]> teacherLessons;
    for (auto& teacher: timetable->teachers)
    {
        for (int i = 0; i < DAYS_PER_WEEK; i++)
        {
            for (int j = 0; j < lessonsPerDay; j++)
                teacherLessons[teacher.first][i].lessonIDs.push_back(-3);
        }
    }
    for (auto& classPair: timetable->classes)
    {
        for (int i = 0; i < DAYS_PER_WEEK; i++)
        {
            for (int j = 0; j < lessonsPerDay; j++)
            {
                for (int k = 0; k < classPair.second.timetableLessons[classPair.second.days[i].timetableLessonIDs[j]].lessonTeacherPairs.size(); k++)
                {
                    LessonTeacherPair& lessonTeacherPair = classPair.second.timetableLessons[classPair.second.days[i].timetableLessonIDs[j]].lessonTeacherPairs[k];
                    teacherLessons[lessonTeacherPair.teacherID][i].lessonIDs[j] = lessonTeacherPair.lessonID;
                }
            }
        }
    }
    return teacherLessons;
}

// I use lessonIDs here in place of non-existant classroomIDs
std::unordered_map<int, WorkDay[DAYS_PER_WEEK]> GetTeacherClassrooms(Timetable* timetable)
{
    std::unordered_map<int, WorkDay[DAYS_PER_WEEK]> teacherClassrooms;
    for (auto& teacher: timetable->teachers)
    {
        for (int i = 0; i < DAYS_PER_WEEK; i++)
        {
            for (int j = 0; j < lessonsPerDay; j++)
                teacherClassrooms[teacher.first][i].lessonIDs.push_back(-3);
        }
    }
    for (auto& classPair: timetable->classes)
    {
        for (int i = 0; i < DAYS_PER_WEEK; i++)
        {
            for (int j = 0; j < lessonsPerDay; j++)
            {
                for (int k = 0; k < classPair.second.timetableLessons[classPair.second.days[i].timetableLessonIDs[j]].lessonTeacherPairs.size(); k++)
                {
                    LessonTeacherPair& lessonTeacherPair = classPair.second.timetableLessons[classPair.second.days[i].timetableLessonIDs[j]].lessonTeacherPairs[k];
                    teacherClassrooms[lessonTeacherPair.teacherID][i].lessonIDs[j] = lessonTeacherPair.classroomID;
                }
            }
        }
    }
    return teacherClassrooms;
}

std::unordered_map<int, WorkDay[DAYS_PER_WEEK]> GetClassroomLessons(Timetable* timetable)
{
    std::unordered_map<int, WorkDay[DAYS_PER_WEEK]> classroomLessons;
    for (auto& teacher: timetable->teachers)
    {
        for (int i = 0; i < DAYS_PER_WEEK; i++)
        {
            for (int j = 0; j < lessonsPerDay; j++)
            classroomLessons[teacher.first][i].lessonIDs.push_back(-3);
        }
    }
    for (auto& classPair: timetable->classes)
    {
        for (int i = 0; i < DAYS_PER_WEEK; i++)
        {
            for (int j = 0; j < lessonsPerDay; j++)
            {
                for (int k = 0; k < classPair.second.timetableLessons[classPair.second.days[i].timetableLessonIDs[j]].lessonTeacherPairs.size(); k++)
                {
                    LessonTeacherPair& lessonTeacherPair = classPair.second.timetableLessons[classPair.second.days[i].timetableLessonIDs[j]].lessonTeacherPairs[k];
                    classroomLessons[lessonTeacherPair.teacherID][i].lessonIDs[j] = lessonTeacherPair.classroomID;
                }
            }
        }
    }
    return classroomLessons;
}

void GetTeacherCollisionErrors(Timetable* timetable)
{
    std::unordered_map<int, Day[DAYS_PER_WEEK]> teacherLessons;
    for (auto& teacher: timetable->teachers)
    {
        for (int i = 0; i < DAYS_PER_WEEK; i++)
        {
            for (int j = 0; j < lessonsPerDay; j++)
                teacherLessons[teacher.first][i].lessons.push_back(false);
        }
    }
    for (auto& classPair: timetable->classes)
    {
        for (int i = 0; i < DAYS_PER_WEEK; i++)
        {
            int counter = 0;
            for (int j = 0; j < classPair.second.days[i].timetableLessonIDs.size(); j++)
            {
                if (classPair.second.days[i].timetableLessonIDs[j] == -1)
                {
                    counter++;
                    continue;
                }
                for (int k = 0; k < classPair.second.timetableLessons[classPair.second.days[i].timetableLessonIDs[j]].lessonTeacherPairs.size(); k++)
                {
                    int& teacherID = classPair.second.timetableLessons[classPair.second.days[i].timetableLessonIDs[j]].lessonTeacherPairs[k].teacherID;
                    if (!teacherLessons[teacherID][i].lessons[counter])
                        teacherLessons[teacherID][i].lessons[counter] = true;
                    else timetable->errors++;
                }
            }
        }
    }
}

void GetClassroomCollisionErrors(Timetable* timetable)
{
    std::unordered_map<int, Day[DAYS_PER_WEEK]> classroomLessons;
    for (auto& classroom: timetable->classrooms)
    {
        for (int i = 0; i < DAYS_PER_WEEK; i++)
        {
            for (int j = 0; j < lessonsPerDay; j++)
                classroomLessons[classroom.first][i].lessons.push_back(false);
        }
    }
    for (auto& classPair: timetable->classes)
    {
        for (int i = 0; i < DAYS_PER_WEEK; i++)
        {
            int counter = 0;
            for (int j = 0; j < classPair.second.days[i].timetableLessonIDs.size(); j++)
            {
                if (classPair.second.days[i].timetableLessonIDs[j] == -1)
                {
                    counter++;
                    continue;
                }
                for (int k = 0; k < classPair.second.timetableLessons[classPair.second.days[i].timetableLessonIDs[j]].lessonTeacherPairs.size(); k++)
                {
                    int& classroomID = classPair.second.timetableLessons[classPair.second.days[i].timetableLessonIDs[j]].lessonTeacherPairs[k].classroomID;
                    if (!classroomLessons[classroomID][i].lessons[counter])
                        classroomLessons[classroomID][i].lessons[counter] = true;
                    else timetable->errors++;
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
        for (int i = 0; i < DAYS_PER_WEEK; i++)
        {
            std::unordered_map<int, int> lessonIntersections;
            for (auto& lesson: classPair.second.timetableLessons)
                lessonIntersections[lesson.first] = 0;
            for (int j = 0; j < classPair.second.days[i].timetableLessonIDs.size(); j++)
            {
                if (classPair.second.days[i].timetableLessonIDs[j] == -1) continue;
                if (++lessonIntersections[classPair.second.days[i].timetableLessonIDs[j]] > 1)
                    totalLessonIntersections[classPair.second.days[i].timetableLessonIDs[j]]++;
            }
        }
        for (auto& lesson: classPair.second.timetableLessons)
        {
            if (totalLessonIntersections[lesson.first] > lesson.second.amount - DAYS_PER_WEEK)
                timetable->errors += totalLessonIntersections[lesson.first] - lesson.second.amount + DAYS_PER_WEEK;
        }
    }
}

void GetTemplateMatchErrors(Timetable* timetable)
{
    std::unordered_map<int, WorkDay[DAYS_PER_WEEK]> teacherLessons = GetTeacherLessons(timetable);
    for (auto& teacher: timetable->teachers)
    {
        for (int i = 0; i < DAYS_PER_WEEK; i++)
        {
            for (int j = 0; j < lessonsPerDay; j++)
            {
                int teacherLesson = teacher.second.workDays[i].lessonIDs[j];
                int classLesson = teacherLessons[teacher.first][i].lessonIDs[j];
                if (teacherLesson == -2) continue;
                else if (teacherLesson == -3 && (classLesson != -3 && classLesson != -2)) timetable->errors++;
                else if (teacherLesson != classLesson) timetable->errors++;
            }
        }
    }
}

void GetFreePeriodErrors(Timetable* timetable)
{
    std::unordered_map<int, int> teacherFreePeriods;
    for (auto& teacher: timetable->teachers)
        teacherFreePeriods[teacher.first] = 0;
    std::unordered_map<int, WorkDay[DAYS_PER_WEEK]> teacherLessons = GetTeacherLessons(timetable);
    for (auto& teacher: timetable->teachers)
    {
        for (int i = 0; i < DAYS_PER_WEEK; i++)
        {
            int lastLesson = timetable->maxLessonID+1;
            int firstLessonIndex = -1;
            int lastLessonIndex = -1;
            for (int j = 0; j < lessonsPerDay; j++)
            {
                if (teacherLessons[teacher.first][i].lessonIDs[j] != -3)
                {
                    firstLessonIndex = j;
                    break;
                }
            }
            for (int j = lessonsPerDay-1; j >= 0; j--)
            {
                if (teacherLessons[teacher.first][i].lessonIDs[j] != -3)
                {
                    lastLessonIndex = j;
                    break;
                }
            }
            if (firstLessonIndex == -1 || lastLessonIndex == -1) continue;
            for (int j = firstLessonIndex; j <= lastLessonIndex; j++)
            {
                int& teacherLesson = teacherLessons[teacher.first][i].lessonIDs[j];
                if (teacherLesson == -3 && lastLesson == -3) teacherFreePeriods[teacher.first]++;
                lastLesson = teacherLesson;
            }
        }
    }
    for (auto& teacher: timetable->teachers)
    {
        if (teacherFreePeriods[teacher.first] < minFreePeriods)
            timetable->errors += minFreePeriods - teacherFreePeriods[teacher.first];
        if (teacherFreePeriods[teacher.first] > minFreePeriods)
            timetable->errors += teacherFreePeriods[teacher.first] - maxFreePeriods;
    }
}

void GetTimetableErrors(Timetable* timetable)
{
    timetable->errors = 0;

    // Get the same teacher in different classrooms at the same time errors
    GetTeacherCollisionErrors(timetable);

    // Get the same classroom being used at the same time errors
    GetClassroomCollisionErrors(timetable);

    // Get the same timetable lesson in a day errors
    GetLessonCollisionErrors(timetable);

    // Get the errors caused by mismatches between teacher selected timetables and classes timetable
    GetTemplateMatchErrors(timetable);

    // Get free period out of bounds errors
    GetFreePeriodErrors(timetable);
}

void GetTeacherMovementBonusPoints(Timetable* timetable)
{
    std::unordered_map<int, WorkDay[DAYS_PER_WEEK]> teacherClassrooms = GetTeacherClassrooms(timetable);
    for (auto& teacher: timetable->teachers)
    {
        for (int i = 0; i < DAYS_PER_WEEK; i++)
        {
            int lastClassroom = timetable->maxLessonID+1;
            int firstClassroomIndex = -1;
            int lastClassroomIndex = -1;
            for (int j = 0; j < lessonsPerDay; j++)
            {
                if (teacherClassrooms[teacher.first][i].lessonIDs[j] != -3)
                {
                    firstClassroomIndex = j;
                    break;
                }
            }
            for (int j = lessonsPerDay-1; j >= 0; j--)
            {
                if (teacherClassrooms[teacher.first][i].lessonIDs[j] != -3)
                {
                    lastClassroomIndex = j;
                    break;
                }
            }
            if (firstClassroomIndex == -1 || lastClassroomIndex == -1) continue;
            for (int j = firstClassroomIndex; j <= lastClassroomIndex; j++)
            {
                int& teacherClassroom = teacherClassrooms[teacher.first][i].lessonIDs[j];
                if (teacherClassroom == lastClassroom) timetable->bonusPoints++;
                lastClassroom = teacherClassroom;
            }
        }
    }
}

void GetStudentMovementBonusPoints(Timetable* timetable)
{
    for (auto& classPair: timetable->classes)
    {
        for (int i = 0; i < DAYS_PER_WEEK; i++)
        {
            int lastClassroom = timetable->maxClassroomID+1;
            for (int j = 0; j < lessonsPerDay; j++)
            {
                if (classPair.second.days[i].timetableLessonIDs[j] < 0) continue;
                for (int k = 0; k < classPair.second.timetableLessons[classPair.second.days[i].timetableLessonIDs[j]].lessonTeacherPairs.size(); k++)
                {
                    LessonTeacherPair& lessonTeacherPair = classPair.second.timetableLessons[classPair.second.days[i].timetableLessonIDs[j]].lessonTeacherPairs[k];
                    if (lastClassroom == lessonTeacherPair.classroomID) timetable->bonusPoints++;
                    lastClassroom = lessonTeacherPair.classroomID;
                }
            }
        }
    }
}

void GetTimetableBonusPoints(Timetable* timetable)
{
    timetable->bonusPoints = 0;

    // Get minimal teacher movement bonus points
    GetTeacherMovementBonusPoints(timetable);

    // Get minimal student movement bonus points
    GetStudentMovementBonusPoints(timetable);
}

void ScoreTimetable(Timetable* timetable)
{
    GetTimetableErrors(timetable);
    GetTimetableBonusPoints(timetable);
}

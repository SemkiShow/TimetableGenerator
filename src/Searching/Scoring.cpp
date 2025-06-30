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
                int timetableLessonID = classPair.second.days[i].classroomLessonPairs[j].timetableLessonID;
                if (timetableLessonID < 0) continue;
                for (int k = 0; k < classPair.second.timetableLessons[timetableLessonID].lessonTeacherPairs.size(); k++)
                {
                    LessonTeacherPair& lessonTeacherPair = classPair.second.timetableLessons[timetableLessonID].lessonTeacherPairs[k];
                    int classroomID = classPair.second.days[i].classroomLessonPairs[j].classroomIDs[k];
                    classroomLessons[lessonTeacherPair.teacherID][i].lessonIDs[j] = classroomID;
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
            for (int j = 0; j < lessonsPerDay; j++)
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
                    #ifdef VERBOSE_LOGGING
                        std::cout << "Teacher collision error. ";
                    #endif
                    }
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
            for (int j = 0; j < lessonsPerDay; j++)
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
                    #ifdef VERBOSE_LOGGING
                        std::cout << "Classroom collision error. ";
                    #endif
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
        int classDays = DAYS_PER_WEEK;
        for (int i = 0; i < DAYS_PER_WEEK; i++)
        {
            std::unordered_map<int, int> lessonIntersections;
            for (auto& lesson: classPair.second.timetableLessons)
                lessonIntersections[lesson.first] = 0;
            bool foundAvailableLesson = false;
            for (int j = 0; j < lessonsPerDay; j++)
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
            #ifdef VERBOSE_LOGGING
                std::cout << "Lesson collision error. ";
            #endif
            }
        }
    }
}

void GetTemplateMatchErrors(Timetable* timetable, std::unordered_map<int, WorkDay[DAYS_PER_WEEK]> teacherLessons)
{
    for (auto& teacher: timetable->teachers)
    {
        for (int i = 0; i < DAYS_PER_WEEK; i++)
        {
            for (int j = 0; j < lessonsPerDay; j++)
            {
                int teacherLesson = teacher.second.workDays[i].lessonIDs[j];
                int classLesson = teacherLessons[teacher.first][i].lessonIDs[j];
                if (teacherLesson == -2) continue;
                else if (teacherLesson == -3 && (classLesson != -3 && classLesson != -2))
                {
                    timetable->errors++;
                #ifdef VERBOSE_LOGGING
                    std::cout << "Template match error. ";
                #endif
                }
                else if (teacherLesson != classLesson)
                {
                    timetable->errors++;
                #ifdef VERBOSE_LOGGING
                    std::cout << "Template match error. ";
                #endif
                }
            }
        }
    }
}

void GetFreePeriodErrors(Timetable* timetable, std::unordered_map<int, WorkDay[DAYS_PER_WEEK]> teacherLessons)
{
    std::unordered_map<int, int> teacherFreePeriods;
    for (auto& teacher: timetable->teachers)
        teacherFreePeriods[teacher.first] = 0;
    for (auto& teacher: timetable->teachers)
    {
        for (int i = 0; i < DAYS_PER_WEEK; i++)
        {
            int firstLessonIndex = -1;
            int lastLessonIndex = -1;
            for (int j = 0; j < lessonsPerDay; j++)
            {
                if (teacherLessons[teacher.first][i].lessonIDs[j] >= 0)
                {
                    firstLessonIndex = j;
                    break;
                }
            }
            for (int j = lessonsPerDay-1; j >= 0; j--)
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
    }
    for (auto& teacher: timetable->teachers)
    {
        if (teacherFreePeriods[teacher.first] < minFreePeriods)
        {
            timetable->errors += minFreePeriods - teacherFreePeriods[teacher.first];
        #ifdef VERBOSE_LOGGING
            std::cout << "Too little teacher free periods error. ";
        #endif
        }
        if (teacherFreePeriods[teacher.first] > minFreePeriods)
        {
            timetable->errors += teacherFreePeriods[teacher.first] - maxFreePeriods;
        #ifdef VERBOSE_LOGGING
            std::cout << "Too many teacher free periods error. ";
        #endif
        }
    }
}

void GetTimetableErrors(Timetable* timetable, std::unordered_map<int, WorkDay[DAYS_PER_WEEK]> teacherLessons)
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
    std::unordered_map<int, WorkDay[DAYS_PER_WEEK]> teacherLessons = GetTeacherLessons(timetable);

    GetTimetableErrors(timetable, teacherLessons);
    GetTimetableBonusPoints(timetable);
}

// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Settings.hpp"
#include "Timetable.hpp"

const size_t classroomsCount = 10;
const size_t lessonsCount = 15;
const size_t teachersCount = 5;
const size_t classesCount = 3;
const size_t timetableLessonsCount = 5;

WorkDay WorkDay::GetRandom()
{
    WorkDay workDay;

    for (size_t i = 0; i < lessonsPerDay; i++)
    {
        workDay.lessonIds.push_back(rand() % lessonsCount);
    }

    return workDay;
}

Classroom Classroom::GetRandom()
{
    const size_t nameSize = 3;

    Classroom classroom;

    for (size_t i = 0; i < nameSize; i++)
    {
        classroom.name += '0' + rand() % 10;
    }

    return classroom;
}

Lesson Lesson::GetRandom()
{
    const size_t nameSize = 7;
    const size_t assignedClassesCount = 4;
    const size_t assignedClassroomsCount = 4;

    Lesson lesson;

    for (size_t i = 0; i < nameSize; i++)
    {
        lesson.name += 'a' + rand() % 26;
    }
    for (size_t i = 0; i < assignedClassesCount; i++)
    {
        lesson.classIds.push_back(rand() % classesCount);
    }
    for (size_t i = 0; i < assignedClassroomsCount; i++)
    {
        lesson.classroomIds.push_back(rand() % classroomsCount);
    }

    return lesson;
}

Teacher Teacher::GetRandom()
{
    const size_t nameSize = 7;
    const size_t assignedLessonsCount = 3;

    Teacher teacher;

    for (size_t i = 0; i < nameSize; i++)
    {
        teacher.name += 'a' + rand() % 26;
    }
    for (size_t i = 0; i < assignedLessonsCount; i++)
    {
        teacher.lessonIds.push_back(rand() % lessonsCount);
    }
    for (size_t i = 0; i < daysPerWeek; i++)
    {
        teacher.workDays.emplace_back(WorkDay::GetRandom());
    }

    return teacher;
}

LessonTeacherPair LessonTeacherPair::GetRandom()
{
    LessonTeacherPair lessonTeacherPair;

    lessonTeacherPair.lessonId = rand() % lessonsCount;
    lessonTeacherPair.teacherId = rand() % teachersCount;

    return lessonTeacherPair;
}

TimetableLesson TimetableLesson::GetRandom()
{
    TimetableLesson timetableLesson;

    timetableLesson.lessonTeacherPairs.push_back(LessonTeacherPair::GetRandom());

    return timetableLesson;
}

ClassroomLessonPair ClassroomLessonPair::GetRandom(Timetable& timetable, int classId)
{
    ClassroomLessonPair classroomLessonPair;

    int timetableLessonId = rand() % timetableLessonsCount;
    classroomLessonPair.timetableLessonId = timetableLessonId;
    for (auto& lessonTeacherPair:
         timetable.classes[classId].timetableLessons[timetableLessonId].lessonTeacherPairs)
    {
        int lessonId = lessonTeacherPair.lessonId;
        auto& classroomIds = timetable.lessons[lessonId].classroomIds;
        int classroomId = classroomIds[rand() % classroomIds.size()];
        classroomLessonPair.classroomIds.push_back(classroomId);
    }

    return classroomLessonPair;
}

Day Day::GetRandom(Timetable& timetable, int classId)
{
    Day day;

    for (size_t i = 0; i < lessonsPerDay; i++)
    {
        day.lessons.push_back(rand() % 2);
        day.classroomLessonPairs.push_back(ClassroomLessonPair::GetRandom(timetable, classId));
    }

    return day;
}

TimetableLessonRule TimetableLessonRule::GetRandom()
{
    TimetableLessonRule timetableLessonRule;

    return timetableLessonRule;
}

Class Class::GetRandom(Timetable& timetable, int classId)
{
    Class group;

    group.number = rand() % classesCount;
    group.letter = 'a' + rand() % 26;
    group.teacherId = rand() % teachersCount;

    // Lessons
    for (size_t i = 0; i < timetableLessonsCount; i++)
    {
        group.timetableLessons[i] = TimetableLesson::GetRandom();
    }

    // Days
    for (size_t i = 0; i < daysPerWeek; i++)
    {
        group.days.emplace_back(Day::GetRandom(timetable, classId));
    }

    return group;
}

Timetable Timetable::GetRandom()
{
    Timetable timetable;

    // Classrooms
    for (size_t i = 0; i < classroomsCount; i++)
    {
        timetable.classrooms[i] = Classroom::GetRandom();
    }

    // Lessons
    for (size_t i = 0; i < lessonsCount; i++)
    {
        timetable.lessons[i] = Lesson::GetRandom();
    }

    // Teachers
    for (size_t i = 0; i < teachersCount; i++)
    {
        timetable.teachers[i] = Teacher::GetRandom();
    }

    // Classes
    const size_t classLettersPerClassNumber = 3;
    for (size_t i = 1; i <= classesCount; i++)
    {
        for (size_t j = 0; j < classLettersPerClassNumber; j++)
        {
            int classId = (i - 1) * classLettersPerClassNumber + j;
            timetable.classes[classId] = Class::GetRandom(timetable, classId);
            timetable.classes[classId].number = i;
            timetable.classes[classId].letter = 'a' + j;
        }
    }

    return timetable;
}

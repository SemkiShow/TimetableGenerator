// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Settings.hpp"
#include "Timetable.hpp"
#include <cstddef>
#include <cstdlib>
#include <string>

constexpr int CLASSROOMS_COUNT = 10;
constexpr int LESSONS_COUNT = 15;
constexpr int TEACHERS_COUNT = 5;
constexpr int CLASSES_COUNT = 3;
constexpr int TIMETABLE_LESSONS_COUNT = 5;
constexpr int DIGITS_SIZE = 10;
constexpr int ALPHABET_SIZE = 26;

WorkDay WorkDay::GetRandom()
{
    WorkDay workDay;

    for (int i = 0; i < g_settings.lessonsPerDay; i++)
    {
        workDay.lessonIds.push_back(rand() % LESSONS_COUNT);
    }

    return workDay;
}

Classroom Classroom::GetRandom()
{
    constexpr int NAME_SIZE = 3;

    Classroom classroom;

    for (size_t i = 0; i < NAME_SIZE; i++)
    {
        classroom.name += char('0' + rand() % DIGITS_SIZE);
    }

    return classroom;
}

Lesson Lesson::GetRandom()
{
    constexpr int NAME_SIZE = 7;
    constexpr int ASSIGNED_CLASSES_COUNT = 4;
    constexpr int ASSIGNED_CLASSROOMS_COUNT = 4;

    Lesson lesson;

    for (size_t i = 0; i < NAME_SIZE; i++)
    {
        lesson.name += char('a' + rand() % ALPHABET_SIZE);
    }
    for (size_t i = 0; i < ASSIGNED_CLASSES_COUNT; i++)
    {
        lesson.classIds.push_back(rand() % CLASSES_COUNT);
    }
    for (size_t i = 0; i < ASSIGNED_CLASSROOMS_COUNT; i++)
    {
        lesson.classroomIds.push_back(rand() % CLASSROOMS_COUNT);
    }

    return lesson;
}

Teacher Teacher::GetRandom()
{
    constexpr int NAME_SIZE = 7;
    constexpr int ASSIGNED_LESSONS_COUNT = 3;

    Teacher teacher;

    for (size_t i = 0; i < NAME_SIZE; i++)
    {
        teacher.name += char('a' + rand() % ALPHABET_SIZE);
    }
    for (size_t i = 0; i < ASSIGNED_LESSONS_COUNT; i++)
    {
        teacher.lessonIds.push_back(rand() % LESSONS_COUNT);
    }
    for (int i = 0; i < g_settings.daysPerWeek; i++)
    {
        teacher.workDays.emplace_back(WorkDay::GetRandom());
    }

    return teacher;
}

LessonTeacherPair LessonTeacherPair::GetRandom()
{
    LessonTeacherPair lessonTeacherPair;

    lessonTeacherPair.lessonId = rand() % LESSONS_COUNT;
    lessonTeacherPair.teacherId = rand() % TEACHERS_COUNT;

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

    int timetableLessonId = rand() % TIMETABLE_LESSONS_COUNT;
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

    for (int i = 0; i < g_settings.lessonsPerDay; i++)
    {
        day.lessons.push_back(rand() % 2 == 0);
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
    Class classPair;

    classPair.number = std::to_string(rand() % CLASSES_COUNT);
    classPair.letter = char('a' + rand() % ALPHABET_SIZE);
    classPair.teacherId = rand() % TEACHERS_COUNT;

    // Lessons
    for (int i = 0; i < TIMETABLE_LESSONS_COUNT; i++)
    {
        classPair.timetableLessons[i] = TimetableLesson::GetRandom();
    }

    // Days
    for (int i = 0; i < g_settings.daysPerWeek; i++)
    {
        classPair.days.emplace_back(Day::GetRandom(timetable, classId));
    }

    return classPair;
}

Timetable Timetable::GetRandom()
{
    Timetable timetable;

    // Classrooms
    for (int i = 0; i < CLASSROOMS_COUNT; i++)
    {
        timetable.classrooms[i] = Classroom::GetRandom();
    }

    // Lessons
    for (int i = 0; i < LESSONS_COUNT; i++)
    {
        timetable.lessons[i] = Lesson::GetRandom();
    }

    // Teachers
    for (int i = 0; i < TEACHERS_COUNT; i++)
    {
        timetable.teachers[i] = Teacher::GetRandom();
    }

    // Classes
    constexpr int CLASS_LETTERS_PER_CLASS_NUMBER = 3;
    for (int i = 1; i <= CLASSES_COUNT; i++)
    {
        for (int j = 0; j < CLASS_LETTERS_PER_CLASS_NUMBER; j++)
        {
            int classId = (i - 1) * CLASS_LETTERS_PER_CLASS_NUMBER + j;
            timetable.classes[classId] = Class::GetRandom(timetable, classId);
            timetable.classes[classId].number = std::to_string(i);
            timetable.classes[classId].letter = char('a' + j);
        }
    }

    return timetable;
}

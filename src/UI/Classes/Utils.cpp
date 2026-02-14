// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Utils.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "UI/Classes.hpp"
#include <algorithm>
#include <vector>

void ResetClassTeacherValues()
{
    classTeacherValues = GetText("none");
    classTeacherValues += '\0';
    classTeacherIds.clear();
    classTeacherIds.push_back(-1);
    for (auto& teacher: currentTimetable.teachers)
    {
        bool isTeacherTaken = false;
        for (auto& classPair: tmpTmpTimetable.classes)
        {
            if (classPair.second.teacherId == teacher.first && classPair.first != currentClassId)
            {
                isTeacherTaken = true;
                break;
            }
        }

        if (!isTeacherTaken)
        {
            if (teacher.second.name == "")
            {
                classTeacherValues += GetText("error");
            }
            else
            {
                classTeacherValues += teacher.second.name;
            }
            classTeacherIds.push_back(teacher.first);
            classTeacherValues += '\0';
        }
    }
    classTeacherValues += '\0';

    classTeacherIndex = 0;
    for (size_t i = 0; i < classTeacherIds.size(); i++)
    {
        if (tmpTmpTimetable.classes[currentClassId].teacherId == classTeacherIds[i])
        {
            classTeacherIndex = i;
            break;
        }
    }
}

bool CompareTimetableLessons(const TimetableLesson lesson1, const TimetableLesson lesson2)
{
    bool areSame = lesson1.lessonTeacherPairs.size() == lesson2.lessonTeacherPairs.size();
    if (!areSame) return false;
    for (size_t i = 0; i < lesson1.lessonTeacherPairs.size(); i++)
    {
        if (lesson1.lessonTeacherPairs[i].lessonId != lesson2.lessonTeacherPairs[i].lessonId ||
            lesson1.lessonTeacherPairs[i].teacherId != lesson2.lessonTeacherPairs[i].teacherId)
        {
            areSame = false;
            break;
        }
    }
    return areSame;
}

void FetchClassLessonsFromSimularClasses(Timetable& timetable, int classId)
{
    // Add missing lessons
    std::string classNumber = timetable.classes[classId].number;
    for (auto& classPair: timetable.classes)
    {
        if (classPair.first == classId) continue;
        if (classPair.second.number != classNumber) continue;

        for (auto& lesson1: classPair.second.timetableLessons)
        {
            bool foundMatchingLesson = false;
            for (auto& lesson2: timetable.classes[classId].timetableLessons)
            {
                if (CompareTimetableLessons(lesson1.second, lesson2.second))
                {
                    foundMatchingLesson = true;
                    break;
                }
            }
            if (!foundMatchingLesson)
            {
                timetable.classes[classId].maxTimetableLessonId++;
                timetable.classes[classId]
                    .timetableLessons[timetable.classes[classId].maxTimetableLessonId] =
                    lesson1.second;

                // Make added lessons support the class
                for (size_t i = 0; i < lesson1.second.lessonTeacherPairs.size(); i++)
                {
                    bool foundMatchingClassId = false;
                    int lessonId = lesson1.second.lessonTeacherPairs[i].lessonId;
                    for (size_t j = 0; j < tmpTmpLessons[lessonId].classIds.size(); j++)
                    {
                        if (tmpTmpLessons[lessonId].classIds[j] == classId)
                        {
                            foundMatchingClassId = true;
                            break;
                        }
                    }
                    if (!foundMatchingClassId)
                    {
                        tmpTmpLessons[lessonId].classIds.push_back(classId);
                    }
                }
            }
        }
    }
}

void ChangeClassesAmount(Timetable& timetable, const std::string& classNumber,
                         const int classesAmount)
{
    int classCounter = 0;
    for (auto it = timetable.classes.begin(); it != timetable.classes.end();)
    {
        if (it->second.number == classNumber)
        {
            if (classCounter >= classesAmount && !newClass)
            {
                timetable.orderedClasses.erase(find(timetable.orderedClasses.begin(),
                                                    timetable.orderedClasses.end(), it->first));
                it = timetable.classes.erase(it);
                continue;
            }
            classCounter++;
        }
        ++it;
    }
    int lastOrderedClassesId = -1;
    for (size_t i = 0; i < timetable.orderedClasses.size(); i++)
    {
        if (timetable.classes[timetable.orderedClasses[i]].number == classNumber)
        {
            lastOrderedClassesId = i;
        }
    }
    for (int i = 0; i < classesAmount - classCounter; i++)
    {
        timetable.maxClassId++;
        timetable.orderedClasses.insert(
            timetable.orderedClasses.begin() + lastOrderedClassesId + i + 1, timetable.maxClassId);
        timetable.classes[timetable.maxClassId] = Class();
        timetable.classes[timetable.maxClassId].number = classNumber;
        FetchClassLessonsFromSimularClasses(tmpTmpTimetable, timetable.maxClassId);
    }
}

void UpdateClassLetters(Timetable& timetable)
{
    int letterCounter = 0;
    std::string lastClassNumber = "";
    for (size_t i = 0; i < timetable.orderedClasses.size(); i++)
    {
        Class& classPair = timetable.classes[timetable.orderedClasses[i]];
        if (classPair.number != lastClassNumber)
        {
            lastClassNumber = classPair.number;
            letterCounter = 0;
        }
        classPair.letter =
            GetNthUtf8Character(GetText("abcdefghijklmnopqrstuvwxyz"), letterCounter++);
    }
}

int GetClassesAmount(Timetable& timetable, const std::string& classNumber)
{
    int output = 0;
    for (auto& classPair: timetable.classes)
    {
        if (classPair.second.number == classNumber) output++;
    }
    return output;
}

void ShiftClass(Timetable& timetable, const int direction,
                const std::vector<std::string> classNumbers, const size_t i)
{
    if ((int)i + direction < 0 || i + direction >= classNumbers.size()) return;

    // Save class teachers
    std::vector<int> classTeachers;
    for (size_t j = 0; j < timetable.orderedClasses.size(); j++)
    {
        Class& classPair = timetable.classes[timetable.orderedClasses[j]];
        if (classPair.number == classNumbers[i])
        {
            classTeachers.push_back(classPair.teacherId);
            classPair.teacherId = -1;
        }
    }

    // Change classes' amount
    int classesAmount = GetClassesAmount(timetable, classNumbers[i]);
    ChangeClassesAmount(timetable, classNumbers[i + direction], classesAmount);

    // Restore class teachers
    int teacherIndexCounter = 0;
    for (size_t j = 0; j < timetable.orderedClasses.size(); j++)
    {
        Class& classPair = timetable.classes[timetable.orderedClasses[j]];
        if (classPair.number == classNumbers[i + direction])
        {
            classPair.teacherId = classTeachers[teacherIndexCounter++];
        }
    }
}

void FixClassTeachers(Timetable& timetable)
{
    // Get all free teacher Ids
    std::vector<int> freeTeachers;
    for (auto& teacher: currentTimetable.teachers)
    {
        freeTeachers.push_back(teacher.first);
    }
    for (auto& classPair: timetable.classes)
    {
        if (classPair.second.teacherId == -1) continue;
        auto it = std::find(freeTeachers.begin(), freeTeachers.end(), classPair.second.teacherId);
        if (it == freeTeachers.end()) continue;
        freeTeachers.erase(it);
    }

    // Remove all incompatible teachers
    for (auto& classPair: timetable.classes)
    {
        if (classPair.second.teacherId == -1) continue;
        bool foundClass = false;
        for (size_t j = 0;
             j < currentTimetable.teachers[classPair.second.teacherId].lessonIds.size(); j++)
        {
            int& lessonId = currentTimetable.teachers[classPair.second.teacherId].lessonIds[j];
            for (size_t k = 0; k < tmpLessons[lessonId].classIds.size(); k++)
            {
                if (classPair.first != tmpLessons[lessonId].classIds[k]) continue;
                foundClass = true;
                break;
            }
            if (foundClass) break;
        }
        if (!foundClass) classPair.second.teacherId = -1;
    }

    // Add teachers to classes with no teacher
    for (size_t i = 0; i < freeTeachers.size(); i++)
    {
        bool foundClass = false;
        for (size_t j = 0; j < currentTimetable.teachers[freeTeachers[i]].lessonIds.size(); j++)
        {
            int& lessonId = currentTimetable.teachers[freeTeachers[i]].lessonIds[j];
            for (size_t k = 0; k < tmpLessons[lessonId].classIds.size(); k++)
            {
                Class& classPair = timetable.classes[tmpLessons[lessonId].classIds[k]];
                if (classPair.teacherId != -1) continue;
                classPair.teacherId = freeTeachers[i];
                foundClass = true;
                break;
            }
            if (foundClass) break;
        }
    }
}

void ShiftClasses(Timetable& timetable, const int direction)
{
    // Update the year
    timetable.year += direction;

    // Collect class numbers in order
    std::vector<std::string> classNumbers;
    if (timetable.orderedClasses.size() > 0)
    {
        classNumbers.push_back(timetable.classes[timetable.orderedClasses[0]].number);
    }
    for (size_t i = 0; i < timetable.orderedClasses.size(); i++)
    {
        auto& classPair = timetable.classes[timetable.orderedClasses[i]];
        if (classPair.number != classNumbers[classNumbers.size() - 1])
        {
            classNumbers.push_back(classPair.number);
        }
    }

    // Change classes' amounts
    if (direction < 0)
    {
        for (size_t i = 0; i < classNumbers.size(); i++)
        {
            ShiftClass(timetable, direction, classNumbers, i);
        }
    }
    if (direction > 0)
    {
        for (int i = classNumbers.size() - 1; i >= 0; i--)
        {
            ShiftClass(timetable, direction, classNumbers, i);
        }
    }

    // Fix class letters
    UpdateClassLetters(timetable);

    // Fix class teachers
    FixClassTeachers(timetable);
}

void LoadTimetableLessonsFromSelection()
{
    for (auto it = tmpTmpTimetable.classes[currentClassId].timetableLessons.begin();
         it != tmpTmpTimetable.classes[currentClassId].timetableLessons.end();)
    {
        if (it->second.lessonTeacherPairs.size() <= 1)
        {
            it = tmpTmpTimetable.classes[currentClassId].timetableLessons.erase(it);
            continue;
        }
        ++it;
    }
    int timetableLessonCounter = 0;
    std::map<int, TimetableLesson> timetableLessons;
    for (auto lesson: tmpTmpTimetable.classes[currentClassId].timetableLessons)
    {
        timetableLessons[timetableLessonCounter++] = lesson.second;
    }
    tmpTmpTimetable.classes[currentClassId].timetableLessons = timetableLessons;
    tmpTmpTimetable.classes[currentClassId].maxTimetableLessonId = timetableLessonCounter - 1;
    for (auto& lesson: tmpLessons)
    {
        if (!classLessons[std::to_string(lesson.first) + "0"]) continue;
        for (auto& teacher: currentTimetable.teachers)
        {
            if (!classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"])
                continue;
            if (!classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"])
                continue;
            if (classLessonAmounts[std::to_string(lesson.first) + teacher.second.name] == 0)
                continue;
            tmpTmpTimetable.classes[currentClassId].maxTimetableLessonId++;
            tmpTmpTimetable.classes[currentClassId]
                .timetableLessons[tmpTmpTimetable.classes[currentClassId].maxTimetableLessonId] =
                TimetableLesson();
            TimetableLesson& timetableLesson =
                tmpTmpTimetable.classes[currentClassId]
                    .timetableLessons[tmpTmpTimetable.classes[currentClassId].maxTimetableLessonId];
            timetableLesson.amount =
                classLessonAmounts[std::to_string(lesson.first) + teacher.second.name];
            timetableLesson.lessonTeacherPairs.push_back(LessonTeacherPair());
            timetableLesson.lessonTeacherPairs[0].lessonId = lesson.first;
            timetableLesson.lessonTeacherPairs[0].teacherId = teacher.first;
        }
    }
}

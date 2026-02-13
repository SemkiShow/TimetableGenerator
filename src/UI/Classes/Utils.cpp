// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Timetable.hpp"
#include "Translations.hpp"
#include "UI/Classes.hpp"
#include <algorithm>
#include <utf8/checked.h>
#include <vector>

void ResetClassTeacherValues()
{
    classTeacherValues = GetText("none");
    classTeacherValues += '\0';
    classTeacherIDs.clear();
    classTeacherIDs.push_back(-1);
    for (auto& teacher: currentTimetable.teachers)
    {
        bool isTeacherTaken = false;
        for (auto& classPair: tmpTmpTimetable.classes)
        {
            if (classPair.second.teacherID == teacher.first && classPair.first != currentClassID)
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
            classTeacherIDs.push_back(teacher.first);
            classTeacherValues += '\0';
        }
    }
    classTeacherValues += '\0';

    classTeacherIndex = 0;
    for (size_t i = 0; i < classTeacherIDs.size(); i++)
    {
        if (tmpTmpTimetable.classes[currentClassID].teacherID == classTeacherIDs[i])
        {
            classTeacherIndex = i;
            break;
        }
    }
}

std::string GetNthUtf8Character(const std::string& utf8String, int index)
{
    auto it = utf8String.begin();
    auto end = utf8String.end();

    for (int i = 0; i < index && it != end; ++i)
        utf8::next(it, end);

    if (it == end) return "";

    auto charStart = it;
    utf8::next(it, end);
    return std::string(charStart, it);
}

bool CompareTimetableLessons(const TimetableLesson lesson1, const TimetableLesson lesson2)
{
    bool areSame = lesson1.lessonTeacherPairs.size() == lesson2.lessonTeacherPairs.size();
    if (!areSame) return false;
    for (size_t i = 0; i < lesson1.lessonTeacherPairs.size(); i++)
    {
        if (lesson1.lessonTeacherPairs[i].lessonID != lesson2.lessonTeacherPairs[i].lessonID ||
            lesson1.lessonTeacherPairs[i].teacherID != lesson2.lessonTeacherPairs[i].teacherID)
        {
            areSame = false;
            break;
        }
    }
    return areSame;
}

void FetchClassLessonsFromSimularClasses(Timetable& timetable, int classID)
{
    // Add missing lessons
    std::string classNumber = timetable.classes[classID].number;
    for (auto& classPair: timetable.classes)
    {
        if (classPair.first == classID) continue;
        if (classPair.second.number != classNumber) continue;

        for (auto& lesson1: classPair.second.timetableLessons)
        {
            bool foundMatchingLesson = false;
            for (auto& lesson2: timetable.classes[classID].timetableLessons)
            {
                if (CompareTimetableLessons(lesson1.second, lesson2.second))
                {
                    foundMatchingLesson = true;
                    break;
                }
            }
            if (!foundMatchingLesson)
            {
                timetable.classes[classID].maxTimetableLessonID++;
                timetable.classes[classID]
                    .timetableLessons[timetable.classes[classID].maxTimetableLessonID] =
                    lesson1.second;

                // Make added lessons support the class
                for (size_t i = 0; i < lesson1.second.lessonTeacherPairs.size(); i++)
                {
                    bool foundMatchingClassID = false;
                    int lessonID = lesson1.second.lessonTeacherPairs[i].lessonID;
                    for (size_t j = 0; j < tmpTmpLessons[lessonID].classIDs.size(); j++)
                    {
                        if (tmpTmpLessons[lessonID].classIDs[j] == classID)
                        {
                            foundMatchingClassID = true;
                            break;
                        }
                    }
                    if (!foundMatchingClassID)
                    {
                        tmpTmpLessons[lessonID].classIDs.push_back(classID);
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
    int lastOrderedClassesID = -1;
    for (size_t i = 0; i < timetable.orderedClasses.size(); i++)
    {
        if (timetable.classes[timetable.orderedClasses[i]].number == classNumber)
        {
            lastOrderedClassesID = i;
        }
    }
    for (int i = 0; i < classesAmount - classCounter; i++)
    {
        timetable.maxClassID++;
        timetable.orderedClasses.insert(
            timetable.orderedClasses.begin() + lastOrderedClassesID + i + 1, timetable.maxClassID);
        timetable.classes[timetable.maxClassID] = Class();
        timetable.classes[timetable.maxClassID].number = classNumber;
        FetchClassLessonsFromSimularClasses(tmpTmpTimetable, timetable.maxClassID);
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
            classTeachers.push_back(classPair.teacherID);
            classPair.teacherID = -1;
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
            classPair.teacherID = classTeachers[teacherIndexCounter++];
        }
    }
}

void FixClassTeachers(Timetable& timetable)
{
    // Get all free teacher IDs
    std::vector<int> freeTeachers;
    for (auto& teacher: currentTimetable.teachers)
    {
        freeTeachers.push_back(teacher.first);
    }
    for (auto& classPair: timetable.classes)
    {
        if (classPair.second.teacherID == -1) continue;
        auto it = std::find(freeTeachers.begin(), freeTeachers.end(), classPair.second.teacherID);
        if (it == freeTeachers.end()) continue;
        freeTeachers.erase(it);
    }

    // Remove all incompatible teachers
    for (auto& classPair: timetable.classes)
    {
        if (classPair.second.teacherID == -1) continue;
        bool foundClass = false;
        for (size_t j = 0;
             j < currentTimetable.teachers[classPair.second.teacherID].lessonIDs.size(); j++)
        {
            int& lessonID = currentTimetable.teachers[classPair.second.teacherID].lessonIDs[j];
            for (size_t k = 0; k < tmpLessons[lessonID].classIDs.size(); k++)
            {
                if (classPair.first != tmpLessons[lessonID].classIDs[k]) continue;
                foundClass = true;
                break;
            }
            if (foundClass) break;
        }
        if (!foundClass) classPair.second.teacherID = -1;
    }

    // Add teachers to classes with no teacher
    for (size_t i = 0; i < freeTeachers.size(); i++)
    {
        bool foundClass = false;
        for (size_t j = 0; j < currentTimetable.teachers[freeTeachers[i]].lessonIDs.size(); j++)
        {
            int& lessonID = currentTimetable.teachers[freeTeachers[i]].lessonIDs[j];
            for (size_t k = 0; k < tmpLessons[lessonID].classIDs.size(); k++)
            {
                Class& classPair = timetable.classes[tmpLessons[lessonID].classIDs[k]];
                if (classPair.teacherID != -1) continue;
                classPair.teacherID = freeTeachers[i];
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
    for (auto it = tmpTmpTimetable.classes[currentClassID].timetableLessons.begin();
         it != tmpTmpTimetable.classes[currentClassID].timetableLessons.end();)
    {
        if (it->second.lessonTeacherPairs.size() <= 1)
        {
            it = tmpTmpTimetable.classes[currentClassID].timetableLessons.erase(it);
            continue;
        }
        ++it;
    }
    int timetableLessonCounter = 0;
    std::map<int, TimetableLesson> timetableLessons;
    for (auto lesson: tmpTmpTimetable.classes[currentClassID].timetableLessons)
    {
        timetableLessons[timetableLessonCounter++] = lesson.second;
    }
    tmpTmpTimetable.classes[currentClassID].timetableLessons = timetableLessons;
    tmpTmpTimetable.classes[currentClassID].maxTimetableLessonID = timetableLessonCounter - 1;
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
            tmpTmpTimetable.classes[currentClassID].maxTimetableLessonID++;
            tmpTmpTimetable.classes[currentClassID]
                .timetableLessons[tmpTmpTimetable.classes[currentClassID].maxTimetableLessonID] =
                TimetableLesson();
            TimetableLesson& timetableLesson =
                tmpTmpTimetable.classes[currentClassID]
                    .timetableLessons[tmpTmpTimetable.classes[currentClassID].maxTimetableLessonID];
            timetableLesson.amount =
                classLessonAmounts[std::to_string(lesson.first) + teacher.second.name];
            timetableLesson.lessonTeacherPairs.push_back(LessonTeacherPair());
            timetableLesson.lessonTeacherPairs[0].lessonID = lesson.first;
            timetableLesson.lessonTeacherPairs[0].teacherID = teacher.first;
        }
    }
}

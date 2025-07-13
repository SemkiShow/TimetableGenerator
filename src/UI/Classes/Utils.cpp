#include "Settings.hpp"
#include "Timetable.hpp"
#include "UI/Classes.hpp"
#include <algorithm>
#include <iostream>
#include <utf8.h>
#include <vector>

void ResetClassTeacherValues()
{
    classTeacherValues = labels["none"];
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
                classTeacherValues += labels["error"];
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
    for (int i = 0; i < classTeacherIDs.size(); i++)
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
    for (int i = 0; i < lesson1.lessonTeacherPairs.size(); i++)
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

void FetchClassLessonsFromSimularClasses(Timetable* timetable, int classID)
{
    // Add missing lessons
    std::string classNumber = timetable->classes[classID].number;
    for (auto& classPair: timetable->classes)
    {
        if (classPair.first == classID) continue;
        if (classPair.second.number != classNumber) continue;

        for (auto& lesson1: classPair.second.timetableLessons)
        {
            bool foundMatchingLesson = false;
            for (auto& lesson2: timetable->classes[classID].timetableLessons)
            {
                if (CompareTimetableLessons(lesson1.second, lesson2.second))
                {
                    foundMatchingLesson = true;
                    break;
                }
            }
            if (!foundMatchingLesson)
            {
                timetable->classes[classID].maxTimetableLessonID++;
                timetable->classes[classID]
                    .timetableLessons[timetable->classes[classID].maxTimetableLessonID] =
                    lesson1.second;

                // Make added lessons support the class
                for (int i = 0; i < lesson1.second.lessonTeacherPairs.size(); i++)
                {
                    bool foundMatchingClassID = false;
                    int lessonID = lesson1.second.lessonTeacherPairs[i].lessonID;
                    for (int j = 0; j < tmpTmpLessons[lessonID].classIDs.size(); j++)
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

void ChangeClassesAmount(Timetable* timetable, const std::string& classNumber,
                         const int classesAmount)
{
    int classCounter = 0;
    for (auto it = timetable->classes.begin(); it != timetable->classes.end();)
    {
        if (it->second.number == classNumber)
        {
            if (classCounter >= classesAmount && !newClass)
            {
                timetable->orderedClasses.erase(find(timetable->orderedClasses.begin(),
                                                     timetable->orderedClasses.end(), it->first));
                it = timetable->classes.erase(it);
                continue;
            }
            classCounter++;
        }
        ++it;
    }
    int lastOrderedClassesID = -1;
    for (int i = 0; i < timetable->orderedClasses.size(); i++)
    {
        if (timetable->classes[timetable->orderedClasses[i]].number == classNumber)
        {
            lastOrderedClassesID = i;
        }
    }
    for (int i = 0; i < classesAmount - classCounter; i++)
    {
        timetable->maxClassID++;
        timetable->orderedClasses.insert(timetable->orderedClasses.begin() + lastOrderedClassesID +
                                             i + 1,
                                         timetable->maxClassID);
        timetable->classes[timetable->maxClassID] = Class();
        timetable->classes[timetable->maxClassID].number = classNumber;
        FetchClassLessonsFromSimularClasses(&tmpTmpTimetable, timetable->maxClassID);
    }
}

void UpdateClassLetters(Timetable* timetable)
{
    int letterCounter = 0;
    std::string lastClassNumber = "";
    for (int i = 0; i < timetable->orderedClasses.size(); i++)
    {
        Class& classPair = timetable->classes[timetable->orderedClasses[i]];
        if (classPair.number != lastClassNumber)
        {
            lastClassNumber = classPair.number;
            letterCounter = 0;
        }
        classPair.letter =
            GetNthUtf8Character(labels["abcdefghijklmnopqrstuvwxyz"], letterCounter++);
    }
}

int GetClassesAmount(Timetable* timetable, const std::string& classNumber)
{
    int output = 0;
    for (auto& classPair: timetable->classes)
    {
        if (classPair.second.number == classNumber) output++;
    }
    return output;
}

void ShiftClass(Timetable* timetable, const int direction,
                const std::vector<std::string> classNumbers, const int i)
{
    if (i + direction < 0 || i + direction >= classNumbers.size()) return;

    std::cout << classNumbers[i] << "->" << classNumbers[i + direction] << '\n';

    // Save class teachers
    std::vector<int> classTeachers;
    for (int j = 0; j < timetable->orderedClasses.size(); j++)
    {
        Class& classPair = timetable->classes[timetable->orderedClasses[j]];
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
    for (int j = 0; j < timetable->orderedClasses.size(); j++)
    {
        Class& classPair = timetable->classes[timetable->orderedClasses[j]];
        std::cout << classPair.number << ", "
                  << (classPair.number == classNumbers[i + direction] ? "true" : "false") << '\n';
        if (classPair.number == classNumbers[i + direction])
        {
            classPair.teacherID = classTeachers[teacherIndexCounter++];
        }
    }
}

void ShiftClasses(Timetable* timetable, const int direction)
{
    // Update the year
    timetable->year += direction;

    // Collect class numbers in order
    std::vector<std::string> classNumbers;
    if (timetable->orderedClasses.size() > 0)
    {
        classNumbers.push_back(timetable->classes[timetable->orderedClasses[0]].number);
    }
    for (int i = 0; i < timetable->orderedClasses.size(); i++)
    {
        auto& classPair = timetable->classes[timetable->orderedClasses[i]];
        if (classPair.number != classNumbers[classNumbers.size() - 1])
        {
            classNumbers.push_back(classPair.number);
        }
    }

    // Change classes' amounts
    if (direction < 0)
    {
        for (int i = 0; i < classNumbers.size(); i++)
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
}

#include "Logging.hpp"
#include "UI.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <utf8.h>

int currentClassID = 0;
bool newClass = false;
bool bulkEditClass = false;
int bulkClassesAmount = 1;
int classTeacherIndex = 0;
std::string classTeacherValues = "";
bool allClassLessons = true;
std::unordered_map<std::string, bool> classLessons;
std::unordered_map<std::string, int> classLessonAmounts;
std::unordered_map<int, bool> allClassLessonTeachers;
std::unordered_map<std::string, bool> classLessonTeachers;
std::vector<bool> allAvailableClassLessonsVertical;
std::vector<bool> allAvailableClassLessonsHorizontal;

static void ResetVariables()
{
    LogInfo("Resetting class variables");
    allAvailableClassLessonsVertical.clear();
    allAvailableClassLessonsVertical.resize(daysPerWeek, true);
    allAvailableClassLessonsHorizontal.clear();
    allAvailableClassLessonsHorizontal.resize(lessonsPerDay, true);

    tmpTmpTimetable.classes[currentClassID].days.resize(daysPerWeek);
    for (int i = 0; i < daysPerWeek; i++)
    {
        if (tmpTmpTimetable.classes[currentClassID].days[i].lessons.size() < lessonsPerDay)
        {
            int iterations = lessonsPerDay - tmpTmpTimetable.classes[currentClassID].days[i].lessons.size();
            for (int j = 0; j < iterations; j++)
                tmpTmpTimetable.classes[currentClassID].days[i].lessons.push_back(newClass);
        }
    }

    allClassLessons = true;
    allClassLessonTeachers.clear();
    for (auto& lesson: currentTimetable.lessons)
        allClassLessonTeachers[lesson.first] = true;
    classLessons.clear();
    classLessonTeachers.clear();

    for (auto& lesson: currentTimetable.lessons)
    {
        classLessons[std::to_string(lesson.first) + "0"] = false;
        for (auto& teacher: currentTimetable.teachers)
        {
            classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"] = false;
            classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"] = newClass;
            classLessonAmounts[std::to_string(lesson.first) + teacher.second.name] = 1;
        }
    }

    for (auto& lesson: currentTimetable.lessons)
    {
        bool classIDFound = false;
        for (int i = 0; i < lesson.second.classIDs.size(); i++)
        {
            if (currentClassID == lesson.second.classIDs[i] ||
                (tmpTmpTimetable.classes[currentClassID].number == tmpTmpTimetable.classes[lesson.second.classIDs[i]].number
                    && bulkEditClass))
            {
                classIDFound = true;
                break;
            }
        }
        if (!classIDFound) continue;
        classLessons[std::to_string(lesson.first) + "0"] = true;
        for (auto& teacher: currentTimetable.teachers)
        {
            bool lessonIDFound = false;
            for (int i = 0; i < teacher.second.lessonIDs.size(); i++)
            {
                if (teacher.second.lessonIDs[i] == lesson.first)
                {
                    lessonIDFound = true;
                    break;
                }
            }
            if (!lessonIDFound) continue;
            classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"] = true;
        }
    }

    for (auto& lesson: currentTimetable.lessons)
    {
        for (auto& teacher: currentTimetable.teachers)
        {
            bool lessonTeacherPairFound = false;
            int lessonTeacherPairID = -1;
            for (auto& timetableLesson: tmpTmpTimetable.classes[currentClassID].timetableLessons)
            {
                if (timetableLesson.second.lessonTeacherPairs.size() != 1) continue;
                if (lesson.first == timetableLesson.second.lessonTeacherPairs[0].lessonID &&
                teacher.first == timetableLesson.second.lessonTeacherPairs[0].teacherID)
                {
                    lessonTeacherPairFound = true;
                    lessonTeacherPairID = timetableLesson.first;
                    break;
                }
            }
            if (!lessonTeacherPairFound) continue;
            classLessonAmounts[std::to_string(lesson.first) + teacher.second.name] =
                tmpTmpTimetable.classes[currentClassID].timetableLessons[lessonTeacherPairID].amount;
            classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"] = true;
        }
    }
}

static int currentLessonID = 0;
bool newCombinedLesson = false;
bool isCombineLessons = false;
void ShowCombineLessons(bool* isOpen)
{
    if (!ImGui::Begin(labels["Combine lessons"].c_str(), isOpen))
    {
        ImGui::End();
        return;
    }

    // Lessons
    ImGui::Columns(2);
    int pushID = 0;
    for (auto& lesson: currentTimetable.lessons)
    {
        if (!classLessons[std::to_string(lesson.first) + "0"]) continue;
        ImGui::PushID(pushID);
        ImGui::Checkbox(lesson.second.name.c_str(), &classLessons[std::to_string(lesson.first) + "2"]);
        ImGui::NextColumn();
        ImGui::PopID();
        pushID++;
        for (auto& teacher: currentTimetable.teachers)
        {
            if (!classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"]) continue;
            ImGui::PushID(pushID);
            ImGui::Checkbox(teacher.second.name.c_str(), &classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "2"]);
            ImGui::PopID();
            pushID++;
        }
        ImGui::NextColumn();
        ImGui::Separator();
    }
    ImGui::Columns(1);

    // Ok and Cancel
    if (ImGui::Button(labels["Ok"].c_str()))
    {
        LogInfo("Pressed the Ok button in combine lessons of class with ID " + std::to_string(currentClassID));
        tmpTmpTimetable.classes[currentClassID].timetableLessons[currentLessonID].lessonTeacherPairs.clear();
        int counter = 0;
        for (auto& lesson: currentTimetable.lessons)
        {
            if (!classLessons[std::to_string(lesson.first) + "2"]) continue;
            for (auto& teacher: currentTimetable.teachers)
            {
                if (!classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "2"]) continue;
                tmpTmpTimetable.classes[currentClassID].timetableLessons[currentLessonID].lessonTeacherPairs.push_back(LessonTeacherPair());
                tmpTmpTimetable.classes[currentClassID].timetableLessons[currentLessonID].lessonTeacherPairs[counter].lessonID =
                    lesson.first;
                tmpTmpTimetable.classes[currentClassID].timetableLessons[currentLessonID].lessonTeacherPairs[counter].teacherID =
                    teacher.first;
                counter++;
            }
        }
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button(labels["Cancel"].c_str()))
    {
        if (newCombinedLesson) tmpTmpTimetable.classes[currentClassID].timetableLessons.erase(currentLessonID);
        *isOpen = false;
    }
    ImGui::End();
}

std::string GetNthUtf8Character(const std::string& utf8_str, int index)
{
    auto it = utf8_str.begin();
    auto end = utf8_str.end();

    for (int i = 0; i < index && it != end; ++i) utf8::next(it, end);

    if (it == end) return "";

    auto char_start = it;
    utf8::next(it, end);
    return std::string(char_start, it);
}

bool isEditClass = false;
void ShowEditClass(bool* isOpen)
{
    if (!ImGui::Begin((newClass ? labels["New class"] : labels["Edit class"]).c_str(), isOpen))
    {
        ImGui::End();
        return;
    }

    // Bulk editing warning
    if (bulkEditClass && !newClass)
    {
        ImGui::TextColored(ImVec4(255, 255, 0, 255), "%s", labels["Warning: you are bulk editing classes!"].c_str());
        ImGui::TextColored(ImVec4(255, 255, 0, 255), "%s", labels["After pressing Ok ALL classes with the number below"].c_str());
        ImGui::TextColored(ImVec4(255, 255, 0, 255), "%s", labels["will be OVERWRITTEN with the data you enter."].c_str());
        ImGui::TextColored(ImVec4(255, 255, 0, 255), "%s", labels["If you don't want that to happen, press the Cancel button."].c_str());
    }

    // Class number
    if (ImGui::InputText(labels["number"].c_str(), &tmpTmpTimetable.classes[currentClassID].number))
        ResetVariables();

    // Classes amount
    if (bulkEditClass)
    {
        ImGui::InputInt(labels["amount"].c_str(), &bulkClassesAmount);
        if (bulkClassesAmount >= labels["abcdefghijklmnopqrstuvwxyz"].size())
            bulkClassesAmount = labels["abcdefghijklmnopqrstuvwxyz"].size()-1;
        if (bulkClassesAmount < 1) bulkClassesAmount = 1;
    }
    // Class letter and teacher
    else
    {
        ImGui::InputText(labels["letter"].c_str(), &tmpTmpTimetable.classes[currentClassID].letter);
        ImGui::Combo(labels["teacher"].c_str(), &classTeacherIndex, classTeacherValues.c_str());
    }
    ImGui::Separator();

    // Class available lessons
    ImGui::Text("%s", labels["available lessons"].c_str());
    ImGui::Separator();
    ImGui::Columns(daysPerWeek + 1);
    ImGui::LabelText("##1", "%s", "");
    ImGui::LabelText("##2", "%s", "");
    int pushID = 3;
    allAvailableClassLessonsHorizontal.resize(lessonsPerDay, true);
    tmpTmpTimetable.classes[currentClassID].days.resize(daysPerWeek);
    for (int i = 0; i < lessonsPerDay; i++)
    {
        ImGui::PushID(pushID);
        bool availableClassLessonsHorizontal = allAvailableClassLessonsHorizontal[i];
        if (ImGui::Checkbox(std::to_string(i).c_str(), &availableClassLessonsHorizontal))
        {
            LogInfo("Clicked allAvailableClassLessonsHorizontal number  " + std::to_string(i) + " in class with ID " + std::to_string(currentClassID));
            allAvailableClassLessonsHorizontal[i] = availableClassLessonsHorizontal;
            for (int j = 0; j < daysPerWeek; j++)
            {
                tmpTmpTimetable.classes[currentClassID].days[j].lessons.resize(lessonsPerDay);
                tmpTmpTimetable.classes[currentClassID].days[j].lessons[i] = allAvailableClassLessonsHorizontal[i];
            }
        }
        ImGui::PopID();
        pushID++;
    }
    ImGui::NextColumn();
    allAvailableClassLessonsVertical.resize(daysPerWeek, false);
    tmpTmpTimetable.classes[currentClassID].days.resize(daysPerWeek);
    for (int i = 0; i < daysPerWeek; i++)
    {
        tmpTmpTimetable.classes[currentClassID].days[i].lessons.resize(lessonsPerDay);
        int weekDay = i;
        while (weekDay >= 7) weekDay -= 7;
        ImGui::Text("%s", weekDays[weekDay].c_str());
        ImGui::PushID(pushID);
        bool availableClassLessonsVertical = allAvailableClassLessonsVertical[i];
        if (ImGui::Checkbox((allAvailableClassLessonsVertical[i] ? labels["Deselect all"] : labels["Select all"]).c_str(), &availableClassLessonsVertical))
        {
            LogInfo("Clicked allAvailableClassLessonsVertical number  " + std::to_string(i) + " in class with ID " + std::to_string(currentClassID));
            allAvailableClassLessonsVertical[i] = availableClassLessonsVertical;
            for (int j = 0; j < lessonsPerDay; j++)
                tmpTmpTimetable.classes[currentClassID].days[i].lessons[j] = allAvailableClassLessonsVertical[i];
        }
        ImGui::PopID();
        pushID++;
        for (int j = 0; j < lessonsPerDay; j++)
        {
            ImGui::PushID(pushID);
            bool isLessonAvailable = tmpTmpTimetable.classes[currentClassID].days[i].lessons[j];
            if (ImGui::Checkbox("", &isLessonAvailable))
            {
                tmpTmpTimetable.classes[currentClassID].days[i].lessons[j] = isLessonAvailable;
                LogInfo("Clicked isLessonAvailable in day " + std::to_string(i) + " in lesson number " + std::to_string(j) +
                    " in class with ID " + std::to_string(currentClassID));
            }
            ImGui::PopID();
            pushID++;
        }
        ImGui::NextColumn();
    }
    ImGui::Columns(1);
    ImGui::Separator();

    // Combine lessons
    ImGui::LabelText("", "%s", labels["lessons"].c_str());
    ImGui::Separator();
    if (ImGui::Button(labels["Combine lessons"].c_str()))
    {
        LogInfo("Clicked the combine lessons button in class with ID " + std::to_string(currentClassID));
        newCombinedLesson = true;
        for (auto& lesson: currentTimetable.lessons)
        {
            if (!classLessons[std::to_string(lesson.first) + "0"]) continue;
            classLessons[std::to_string(lesson.first) + "2"] = false;
            for (auto& teacher: currentTimetable.teachers)
            {
                if (!classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"]) continue;
                classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "2"] = false;
            }
        }
        tmpTmpTimetable.classes[currentClassID].maxTimetableLessonID++;
        currentLessonID = tmpTmpTimetable.classes[currentClassID].maxTimetableLessonID;
        tmpTmpTimetable.classes[currentClassID].timetableLessons[currentLessonID] = TimetableLesson();
        isCombineLessons = true;
    }
    for (auto it = tmpTmpTimetable.classes[currentClassID].timetableLessons.begin(); it != tmpTmpTimetable.classes[currentClassID].timetableLessons.end();)
    {
        if (it->second.lessonTeacherPairs.size() <= 1)
        {
            ++it;
            continue;
        }
        ImGui::PushID(pushID);
        if (ImGui::Button(labels["-"].c_str()))
        {
            LogInfo("Removed class with ID " + std::to_string(it->first));
            ImGui::PopID();
            pushID++;
            it = tmpTmpTimetable.classes[currentClassID].timetableLessons.erase(it);
            break;
        }
        ImGui::SameLine();
        if (ImGui::Button(labels["Edit"].c_str()))
        {
            LogInfo("Editing class with ID " + std::to_string(currentClassID));
            newCombinedLesson = false;
            for (auto& lesson: currentTimetable.lessons)
            {
                if (!classLessons[std::to_string(lesson.first) + "0"]) continue;
                classLessons[std::to_string(lesson.first) + "2"] = false;
                for (auto& teacher: currentTimetable.teachers)
                {
                    if (!classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"]) continue;
                    classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "2"] = false;
                }
            }
            for (int j = 0; j < it->second.lessonTeacherPairs.size(); j++)
            {
                if (!classLessons[std::to_string(it->second.lessonTeacherPairs[j].lessonID) + "0"]) continue;
                if (!classLessonTeachers[std::to_string(it->second.lessonTeacherPairs[j].lessonID) +
                    currentTimetable.teachers[it->second.lessonTeacherPairs[j].teacherID].name + "0"]) continue;
                classLessons[std::to_string(it->second.lessonTeacherPairs[j].lessonID) + "2"] = true;
                classLessonTeachers[std::to_string(it->second.lessonTeacherPairs[j].lessonID) +
                    currentTimetable.teachers[it->second.lessonTeacherPairs[j].teacherID].name + "2"] = true;
            }
            currentLessonID = it->first;
            isCombineLessons = true;
        }
        ImGui::SameLine();
        std::string text = "";
        for (int j = 0; j < it->second.lessonTeacherPairs.size(); j++)
        {
            text += currentTimetable.lessons[it->second.lessonTeacherPairs[j].lessonID].name + " (";
            text += currentTimetable.teachers[it->second.lessonTeacherPairs[j].teacherID].name + ")";
            if (j < it->second.lessonTeacherPairs.size()-1) text += ", ";
        }
        ImGui::InputInt(text.c_str(), &it->second.amount);
        ImGui::PopID();
        pushID++;
        ++it;
    }
    ImGui::Separator();
    ImGui::Columns(2);

    // Lessons
    ImGui::LabelText("##3", "%s", "");
    for (auto& lesson: currentTimetable.lessons)
    {
        if (!classLessons[std::to_string(lesson.first) + "0"]) continue;
        ImGui::PushID(pushID);
        ImGui::NextColumn();
        if (ImGui::Checkbox((allClassLessonTeachers[lesson.first] ? labels["Deselect all"] + "##1" : labels["Select all"] + "##1").c_str(),
        &allClassLessonTeachers[lesson.first]))
        {
            LogInfo("Clicked allClassLessonTeachers in class with ID " + std::to_string(currentClassID));
            for (auto& teacher: currentTimetable.teachers)
                classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"] =
                allClassLessonTeachers[lesson.first];
        }
        ImGui::NextColumn();
        ImGui::PopID();
        pushID++;
        for (auto& teacher: currentTimetable.teachers)
        {
            if (!classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"]) continue;
            ImGui::PushID(pushID);
            ImGui::InputInt(lesson.second.name.c_str(), &classLessonAmounts[std::to_string(lesson.first) + teacher.second.name]);
            if (classLessonAmounts[std::to_string(lesson.first) + teacher.second.name] < 0)
                classLessonAmounts[std::to_string(lesson.first) + teacher.second.name] = 0;
            ImGui::NextColumn();
            ImGui::Checkbox(teacher.second.name.c_str(),
                &classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"]);
            ImGui::NextColumn();
            ImGui::PopID();
            pushID++;
        }
        ImGui::Separator();
    }
    ImGui::Columns(1);

    // Ok and Cancel
    if (ImGui::Button(labels["Ok"].c_str()))
    {
        LogInfo("Clicked the Ok button while editing class with ID " + std::to_string(currentClassID));
        for (auto it = tmpTmpTimetable.classes[currentClassID].timetableLessons.begin(); it != tmpTmpTimetable.classes[currentClassID].timetableLessons.end();)
        {
            if (it->second.lessonTeacherPairs.size() <= 1)
            {
                it = tmpTmpTimetable.classes[currentClassID].timetableLessons.erase(it);
                continue;
            }
            ++it;
        }
        int timetableLessonCounter = 0;
        for (auto it = tmpTmpTimetable.classes[currentClassID].timetableLessons.begin(); it != tmpTmpTimetable.classes[currentClassID].timetableLessons.end();)
        {
            tmpTmpTimetable.classes[currentClassID].timetableLessons[timetableLessonCounter] = it->second;
            it = tmpTmpTimetable.classes[currentClassID].timetableLessons.erase(it);
            timetableLessonCounter++;
        }
        tmpTmpTimetable.classes[currentClassID].maxTimetableLessonID = timetableLessonCounter-1;
        for (auto& lesson: currentTimetable.lessons)
        {
            if (!classLessons[std::to_string(lesson.first) + "0"]) continue;
            for (auto& teacher: currentTimetable.teachers)
            {
                if (!classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"]) continue;
                if (!classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"]) continue;
                if (classLessonAmounts[std::to_string(lesson.first) + teacher.second.name] == 0) continue;
                tmpTmpTimetable.classes[currentClassID].maxTimetableLessonID++;
                tmpTmpTimetable.classes[currentClassID].timetableLessons[tmpTmpTimetable.classes[currentClassID].maxTimetableLessonID] =
                    TimetableLesson();
                tmpTmpTimetable.classes[currentClassID].timetableLessons[tmpTmpTimetable.classes[currentClassID].maxTimetableLessonID].amount =
                    classLessonAmounts[std::to_string(lesson.first) + teacher.second.name];
                tmpTmpTimetable.classes[currentClassID].timetableLessons[tmpTmpTimetable.classes[currentClassID].maxTimetableLessonID].lessonTeacherPairs
                    .push_back(LessonTeacherPair());
                tmpTmpTimetable.classes[currentClassID].timetableLessons[tmpTmpTimetable.classes[currentClassID].maxTimetableLessonID].lessonTeacherPairs[0].lessonID =
                    lesson.first;
                tmpTmpTimetable.classes[currentClassID].timetableLessons[tmpTmpTimetable.classes[currentClassID].maxTimetableLessonID].lessonTeacherPairs[0].teacherID =
                    teacher.first;
            }
        }
        if (bulkEditClass)
        {
            int classCounter = 0;
            for (auto it = tmpTmpTimetable.classes.begin(); it != tmpTmpTimetable.classes.end();)
            {
                if (it->second.number == tmpTmpTimetable.classes[currentClassID].number && it->first != currentClassID)
                {
                    if (classCounter >= bulkClassesAmount && !newClass)
                    {
                        tmpTmpTimetable.orderedClasses.erase(
                            find(tmpTmpTimetable.orderedClasses.begin(), tmpTmpTimetable.orderedClasses.end(), it->first));
                        it = tmpTmpTimetable.classes.erase(it);
                        classCounter--;
                        continue;
                    }
                    classCounter++;
                }
                ++it;
            }
            int orderedClassesID = -1;
            for (int i = 0; i < tmpTmpTimetable.orderedClasses.size(); i++)
            {
                if (tmpTmpTimetable.orderedClasses[i] == currentClassID)
                {
                    orderedClassesID = i;
                }
            }
            for (int i = 0; i < bulkClassesAmount-1 - classCounter; i++)
            {
                tmpTmpTimetable.maxClassID++;
                tmpTmpTimetable.orderedClasses.insert(tmpTmpTimetable.orderedClasses.begin() + orderedClassesID + i + 1, tmpTmpTimetable.maxClassID);
                tmpTmpTimetable.classes[tmpTmpTimetable.maxClassID] = Class();
                tmpTmpTimetable.classes[tmpTmpTimetable.maxClassID].number = tmpTmpTimetable.classes[currentClassID].number;
            }
            for (auto& classPair: tmpTmpTimetable.classes)
            {
                if (classPair.first == currentClassID) continue;
                if (classPair.second.number == tmpTmpTimetable.classes[currentClassID].number)
                {
                    int teacherID = classPair.second.teacherID;
                    classPair.second = tmpTmpTimetable.classes[currentClassID];
                    classPair.second.teacherID = teacherID;
                }
            }
            for (int i = 0; i < bulkClassesAmount; i++)
            {
                tmpTmpTimetable.classes[tmpTmpTimetable.orderedClasses[orderedClassesID + i]].letter =
                    GetNthUtf8Character(labels["abcdefghijklmnopqrstuvwxyz"], i);
            }
        }
        else
        {
            for (auto& teacher: currentTimetable.teachers)
            {
                if (classTeacherIndex <= 0)
                {
                    tmpTmpTimetable.classes[currentClassID].teacherID = teacher.first;
                    break;
                }
                classTeacherIndex--;
            }
        }
        tmpTimetable.classes = tmpTmpTimetable.classes;
        tmpTimetable.maxClassID = tmpTmpTimetable.maxClassID;
        tmpTimetable.orderedClasses = tmpTmpTimetable.orderedClasses;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button(labels["Cancel"].c_str())) *isOpen = false;
    ImGui::End();
}

bool isClasses = false;
void ShowClasses(bool* isOpen)
{
    if (!ImGui::Begin(labels["Classes"].c_str(), isOpen))
    {
        ImGui::End();
        return;
    }

    if (ImGui::Button(labels["+"].c_str()))
    {
        newClass = true;
        bulkEditClass = true;
        tmpTmpTimetable.classes = tmpTimetable.classes;
        tmpTmpTimetable.maxClassID = tmpTimetable.maxClassID;
        tmpTmpTimetable.orderedClasses = tmpTimetable.orderedClasses;
        tmpTmpTimetable.maxClassID++;
        currentClassID = tmpTmpTimetable.maxClassID;
        LogInfo("Adding a new class with ID " + std::to_string(currentClassID));
        tmpTmpTimetable.orderedClasses.push_back(currentClassID);
        tmpTmpTimetable.classes[currentClassID] = Class();
        tmpTmpTimetable.classes[currentClassID].number = "0";
        ResetVariables();
        isEditClass = true;
    }
    ImGui::Separator();
    ImGui::Columns(2);

    std::string lastClassNumber = "";
    int buttonID = 0;
    for (int i = 0; i < tmpTimetable.orderedClasses.size(); i++)
    {
        if (lastClassNumber != tmpTimetable.classes[tmpTimetable.orderedClasses[i]].number)
        {
            lastClassNumber = tmpTimetable.classes[tmpTimetable.orderedClasses[i]].number;
            ImGui::PushID(buttonID);

            if (ImGui::Button(labels["-"].c_str()))
            {
                LogInfo("Removed classes with number " + lastClassNumber);
                ImGui::PopID();
                for (auto it = tmpTimetable.classes.begin(); it != tmpTimetable.classes.end();)
                {
                    if (it->second.number == lastClassNumber)
                    {
                        tmpTimetable.orderedClasses.erase(find(tmpTimetable.orderedClasses.begin(), tmpTimetable.orderedClasses.end(), it->first));
                        it = tmpTimetable.classes.erase(it);
                        continue;
                    }
                    ++it;
                }
                break;
            }
            ImGui::SameLine();

            if (ImGui::Button(labels["Edit"].c_str()))
            {
                LogInfo("Bulk editing classes with number " + lastClassNumber);
                bulkClassesAmount = 0;
                for (auto& classPair: tmpTimetable.classes)
                {
                    if (classPair.second.number == lastClassNumber) bulkClassesAmount++;
                }
                newClass = false;
                bulkEditClass = true;
                tmpTmpTimetable.classes = tmpTimetable.classes;
                tmpTmpTimetable.maxClassID = tmpTimetable.maxClassID;
                tmpTmpTimetable.orderedClasses = tmpTimetable.orderedClasses;
                currentClassID = tmpTmpTimetable.orderedClasses[i];
                ResetVariables();
                isEditClass = true;
            }
            ImGui::SameLine();

            ImGui::Text("%s", lastClassNumber.c_str());
            ImGui::Indent();
            if (ImGui::Button(labels["+"].c_str()))
            {
                newClass = true;
                bulkEditClass = false;
                for (int j = 0; j < tmpTimetable.orderedClasses.size(); j++)
                {
                    if (tmpTimetable.classes[tmpTimetable.orderedClasses[j]].number == lastClassNumber) currentClassID = j;
                }
                currentClassID++;
                tmpTmpTimetable.classes = tmpTimetable.classes;
                tmpTmpTimetable.maxClassID = tmpTimetable.maxClassID;
                tmpTmpTimetable.orderedClasses = tmpTimetable.orderedClasses;
                tmpTmpTimetable.maxClassID++;
                tmpTmpTimetable.orderedClasses.insert(tmpTmpTimetable.orderedClasses.begin() + currentClassID, tmpTmpTimetable.maxClassID);
                tmpTmpTimetable.classes[tmpTmpTimetable.maxClassID] = Class();
                tmpTmpTimetable.classes[tmpTmpTimetable.maxClassID].number = tmpTmpTimetable.classes[tmpTmpTimetable.orderedClasses[currentClassID-1]].number;
                currentClassID = tmpTmpTimetable.maxClassID;
                LogInfo("Adding a new class with number " + lastClassNumber + " and ID " + std::to_string(currentClassID));
                ResetVariables();
                isEditClass = true;
            }
            ImGui::Unindent();
            ImGui::NextColumn();
            ImGui::LabelText("", "%s", "");
            ImGui::NextColumn();
            ImGui::PopID();
            buttonID++;
        }
        ImGui::Indent();
        ImGui::PushID(buttonID);

        if (ImGui::Button(labels["-"].c_str()))
        {
            LogInfo("Removed a class with ID " + std::to_string(tmpTimetable.orderedClasses[i]));
            tmpTimetable.classes.erase(tmpTimetable.orderedClasses[i]);
            tmpTimetable.orderedClasses.erase(tmpTimetable.orderedClasses.begin() + i);
            i--;
        }
        ImGui::SameLine();

        if (ImGui::Button(labels["Edit"].c_str()))
        {
            newClass = false;
            bulkEditClass = false;
            tmpTmpTimetable.classes = tmpTimetable.classes;
            tmpTmpTimetable.maxClassID = tmpTimetable.maxClassID;
            tmpTmpTimetable.orderedClasses = tmpTimetable.orderedClasses;
            currentClassID = tmpTmpTimetable.orderedClasses[i];
            LogInfo("Editing class with ID " + std::to_string(currentClassID));
            ResetVariables();
            isEditClass = true;
        }
        ImGui::SameLine();

        ImGui::Text("%s", (tmpTimetable.classes[tmpTimetable.orderedClasses[i]].number + tmpTimetable.classes[tmpTimetable.orderedClasses[i]].letter).c_str());
        ImGui::PopID();
        buttonID++;
        ImGui::Unindent();
        ImGui::NextColumn();

        if (currentTimetable.teachers.find(tmpTimetable.classes[tmpTimetable.orderedClasses[i]].teacherID) !=
        currentTimetable.teachers.end())
            ImGui::LabelText("", "%s", currentTimetable.teachers[tmpTimetable.classes[tmpTimetable.orderedClasses[i]].teacherID].name.c_str());
        ImGui::NextColumn();
    }
    ImGui::Columns(1);
    ImGui::Separator();

    // Ok and Cancel
    if (ImGui::Button(labels["Ok"].c_str()))
    {
        LogInfo("Clicked Ok in the classes menu");
        currentTimetable.classes = tmpTimetable.classes;
        currentTimetable.maxClassID = tmpTimetable.maxClassID;
        currentTimetable.orderedClasses = tmpTimetable.orderedClasses;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button(labels["Cancel"].c_str())) *isOpen = false;
    ImGui::End();
}

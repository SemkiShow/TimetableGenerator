#include "UI/Classes.hpp"
#include "Logging.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"
#include "UI.hpp"
#include <algorithm>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <unordered_map>

int currentClassID = 0;
bool newClass = false;
bool bulkEditClass = false;
int bulkClassesAmount = 1;
int classTeacherIndex = 0;
std::string classTeacherValues = "";
std::vector<int> classTeacherIDs;
bool allClassLessons = true;
std::unordered_map<std::string, bool> classLessons;
std::unordered_map<std::string, int> classLessonAmounts;
std::unordered_map<int, bool> allClassLessonTeachers;
std::unordered_map<std::string, bool> classLessonTeachers;
std::vector<bool> allAvailableClassLessonsVertical;
std::vector<bool> allAvailableClassLessonsHorizontal;
std::map<int, Lesson> tmpLessons;
std::map<int, Lesson> tmpTmpLessons;

static void ResetVariables()
{
    LogInfo("Resetting class variables");
    allAvailableClassLessonsVertical.clear();
    allAvailableClassLessonsVertical.resize(daysPerWeek, true);
    allAvailableClassLessonsHorizontal.clear();
    allAvailableClassLessonsHorizontal.resize(lessonsPerDay, true);

    tmpTmpTimetable.classes[currentClassID].days.resize(daysPerWeek);
    for (size_t i = 0; i < daysPerWeek; i++)
    {
        if (tmpTmpTimetable.classes[currentClassID].days[i].lessons.size() < lessonsPerDay)
        {
            int iterations =
                lessonsPerDay - tmpTmpTimetable.classes[currentClassID].days[i].lessons.size();
            for (int j = 0; j < iterations; j++)
                tmpTmpTimetable.classes[currentClassID].days[i].lessons.push_back(newClass);
        }
    }

    allClassLessons = true;
    allClassLessonTeachers.clear();
    for (auto& lesson: tmpLessons)
        allClassLessonTeachers[lesson.first] = true;
    classLessons.clear();
    classLessonTeachers.clear();

    for (auto& lesson: tmpLessons)
    {
        classLessons[std::to_string(lesson.first) + "0"] = false;
        for (auto& teacher: currentTimetable.teachers)
        {
            classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"] = false;
            classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"] =
                newClass;
            classLessonAmounts[std::to_string(lesson.first) + teacher.second.name] = 1;
        }
    }

    for (auto& lesson: tmpLessons)
    {
        bool classIDFound = false;
        for (size_t i = 0; i < lesson.second.classIDs.size(); i++)
        {
            if (currentClassID == lesson.second.classIDs[i] ||
                (tmpTmpTimetable.classes[currentClassID].number ==
                     tmpTmpTimetable.classes[lesson.second.classIDs[i]].number &&
                 bulkEditClass))
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
            for (size_t i = 0; i < teacher.second.lessonIDs.size(); i++)
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

    for (auto& lesson: tmpLessons)
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
                tmpTmpTimetable.classes[currentClassID]
                    .timetableLessons[lessonTeacherPairID]
                    .amount;
            classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"] = true;
        }
    }

    tmpTmpLessons = tmpLessons;
    ResetClassTeacherValues();
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
        ImGui::TextColored(ImVec4(255, 255, 0, 255), "%s",
                           labels["Warning: you are bulk editing classes!"].c_str());
        ImGui::TextColored(ImVec4(255, 255, 0, 255), "%s",
                           labels["After pressing Ok ALL classes with the number below"].c_str());
        ImGui::TextColored(ImVec4(255, 255, 0, 255), "%s",
                           labels["will be OVERWRITTEN with the data you enter."].c_str());
        ImGui::TextColored(
            ImVec4(255, 255, 0, 255), "%s",
            labels["If you don't want that to happen, press the Cancel button."].c_str());
    }

    // Class number
    if (ImGui::InputText(labels["number"].c_str(), &tmpTmpTimetable.classes[currentClassID].number))
    {
        tmpTmpTimetable.classes[currentClassID].timetableLessons.clear();
        tmpTmpTimetable.classes[currentClassID].maxTimetableLessonID = 0;
        ResetVariables();
        FetchClassLessonsFromSimularClasses(&tmpTmpTimetable, currentClassID);
    }

    // Classes amount
    if (bulkEditClass)
    {
        ImGui::InputInt(labels["amount"].c_str(), &bulkClassesAmount);
        if (bulkClassesAmount < 1) bulkClassesAmount = 1;
        if ((size_t)bulkClassesAmount >= labels["abcdefghijklmnopqrstuvwxyz"].size())
            bulkClassesAmount = labels["abcdefghijklmnopqrstuvwxyz"].size() - 1;
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
    for (size_t i = 0; i < lessonsPerDay; i++)
    {
        ImGui::PushID(pushID++);
        bool availableClassLessonsHorizontal = allAvailableClassLessonsHorizontal[i];
        if (ImGui::Checkbox(std::to_string(i).c_str(), &availableClassLessonsHorizontal))
        {
            LogInfo("Clicked allAvailableClassLessonsHorizontal number  " + std::to_string(i) +
                    " in class with ID " + std::to_string(currentClassID));
            allAvailableClassLessonsHorizontal[i] = availableClassLessonsHorizontal;
            for (size_t j = 0; j < daysPerWeek; j++)
            {
                tmpTmpTimetable.classes[currentClassID].days[j].lessons.resize(lessonsPerDay);
                tmpTmpTimetable.classes[currentClassID].days[j].lessons[i] =
                    allAvailableClassLessonsHorizontal[i];
            }
        }
        ImGui::PopID();
        pushID++;
    }
    ImGui::NextColumn();
    allAvailableClassLessonsVertical.resize(daysPerWeek, false);
    tmpTmpTimetable.classes[currentClassID].days.resize(daysPerWeek);
    for (size_t i = 0; i < daysPerWeek; i++)
    {
        tmpTmpTimetable.classes[currentClassID].days[i].lessons.resize(lessonsPerDay);
        int weekDay = i;
        while (weekDay >= 7)
            weekDay -= 7;
        ImGui::Text("%s", weekDays[weekDay].c_str());
        ImGui::PushID(pushID++);
        bool availableClassLessonsVertical = allAvailableClassLessonsVertical[i];
        if (ImGui::Checkbox((allAvailableClassLessonsVertical[i] ? labels["Deselect all"]
                                                                 : labels["Select all"])
                                .c_str(),
                            &availableClassLessonsVertical))
        {
            LogInfo("Clicked allAvailableClassLessonsVertical number  " + std::to_string(i) +
                    " in class with ID " + std::to_string(currentClassID));
            allAvailableClassLessonsVertical[i] = availableClassLessonsVertical;
            for (size_t j = 0; j < lessonsPerDay; j++)
                tmpTmpTimetable.classes[currentClassID].days[i].lessons[j] =
                    allAvailableClassLessonsVertical[i];
        }
        ImGui::PopID();
        for (size_t j = 0; j < lessonsPerDay; j++)
        {
            ImGui::PushID(pushID++);
            bool isLessonAvailable = tmpTmpTimetable.classes[currentClassID].days[i].lessons[j];
            if (ImGui::Checkbox("", &isLessonAvailable))
            {
                tmpTmpTimetable.classes[currentClassID].days[i].lessons[j] = isLessonAvailable;
                LogInfo("Clicked isLessonAvailable in day " + std::to_string(i) +
                        " in lesson number " + std::to_string(j) + " in class with ID " +
                        std::to_string(currentClassID));
            }
            ImGui::PopID();
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
        LogInfo("Clicked the combine lessons button in class with ID " +
                std::to_string(currentClassID));
        newCombinedLesson = true;
        ResetCombineLessonsVariables();
        tmpTmpTimetable.classes[currentClassID].maxTimetableLessonID++;
        currentLessonID = tmpTmpTimetable.classes[currentClassID].maxTimetableLessonID;
        tmpTmpTimetable.classes[currentClassID].timetableLessons[currentLessonID] =
            TimetableLesson();
        isCombineLessons = true;
    }
    for (auto it = tmpTmpTimetable.classes[currentClassID].timetableLessons.begin();
         it != tmpTmpTimetable.classes[currentClassID].timetableLessons.end();)
    {
        if (it->second.lessonTeacherPairs.size() <= 1)
        {
            ++it;
            continue;
        }
        ImGui::PushID(pushID++);
        if (ImGui::Button(labels["-"].c_str()))
        {
            LogInfo("Removed a timetable lesson with ID " + std::to_string(it->first) +
                    " in a class with ID " + std::to_string(it->first));
            ImGui::PopID();
            it = tmpTmpTimetable.classes[currentClassID].timetableLessons.erase(it);
            break;
        }
        ImGui::SameLine();
        if (ImGui::Button(labels["Edit"].c_str()))
        {
            LogInfo("Editing a timetable lesson with ID " + std::to_string(it->first) +
                    " in a class with ID " + std::to_string(currentClassID));
            newCombinedLesson = false;
            currentLessonID = it->first;
            ResetCombineLessonsVariables();
            isCombineLessons = true;
        }
        ImGui::SameLine();
        std::string text = "";
        for (size_t j = 0; j < it->second.lessonTeacherPairs.size(); j++)
        {
            text += tmpLessons[it->second.lessonTeacherPairs[j].lessonID].name + " (";
            text +=
                currentTimetable.teachers[it->second.lessonTeacherPairs[j].teacherID].name + ")";
            if (j < it->second.lessonTeacherPairs.size() - 1) text += "\n";
        }
        ImGui::InputInt(text.c_str(), &it->second.amount);
        ImGui::PopID();
        pushID++;
        ++it;
    }
    ImGui::Separator();
    ImGui::Columns(2);

    // Lessons
    for (auto& lesson: tmpLessons)
    {
        if (!classLessons[std::to_string(lesson.first) + "0"]) continue;
        bool anyTeacherSelected = false;
        for (auto& teacher: currentTimetable.teachers)
        {
            if (!classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"])
                continue;
            if (classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"])
            {
                anyTeacherSelected = true;
                break;
            }
        }
        if (!anyTeacherSelected)
        {
            ImGui::PushID(pushID++);
            ImGui::TextColored(ImVec4(255, 255, 0, 255), "%s",
                               labels["Warning: no teacher selected for this lesson"].c_str());
            ImGui::PopID();
        }
        ImGui::PushID(pushID++);
        ImGui::NextColumn();
        if (ImGui::Checkbox((allClassLessonTeachers[lesson.first] ? labels["Deselect all"] + "##1"
                                                                  : labels["Select all"] + "##1")
                                .c_str(),
                            &allClassLessonTeachers[lesson.first]))
        {
            LogInfo("Clicked allClassLessonTeachers in class with ID " +
                    std::to_string(currentClassID));
            for (auto& teacher: currentTimetable.teachers)
                classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"] =
                    allClassLessonTeachers[lesson.first];
        }
        ImGui::NextColumn();
        ImGui::PopID();
        for (auto& teacher: currentTimetable.teachers)
        {
            if (!classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"])
                continue;
            ImGui::PushID(pushID++);
            ImGui::BeginDisabled(
                !classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"]);
            ImGui::InputInt(
                lesson.second.name.c_str(),
                &classLessonAmounts[std::to_string(lesson.first) + teacher.second.name]);
            if (classLessonAmounts[std::to_string(lesson.first) + teacher.second.name] < 0)
                classLessonAmounts[std::to_string(lesson.first) + teacher.second.name] = 0;
            ImGui::EndDisabled();
            ImGui::NextColumn();
            ImGui::Checkbox(
                teacher.second.name.c_str(),
                &classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"]);
            ImGui::NextColumn();
            ImGui::PopID();
        }
        ImGui::Separator();
    }
    ImGui::Columns(1);

    // Additional rules
    if (ImGui::Button(labels["Add a lesson rule"].c_str()))
    {
        newRule = true;
        tmpTmpTimetable.classes[currentClassID].timetableLessonRules.push_back(
            TimetableLessonRule());
        currentRuleID = tmpTmpTimetable.classes[currentClassID].timetableLessonRules.size() - 1;
        ResetRulesVariables();
        isRules = true;
    }
    ImGui::Columns(3);
    ImGui::Text("%s", labels["Rules"].c_str());
    ImGui::NextColumn();
    ImGui::Text("%s", labels["Preserve order"].c_str());
    ImGui::NextColumn();
    ImGui::Text("%s", labels["Amount"].c_str());
    ImGui::NextColumn();
    ImGui::Separator();
    for (size_t i = 0; i < tmpTmpTimetable.classes[currentClassID].timetableLessonRules.size(); i++)
    {
        if (isRules && newRule && i == currentRuleID) continue;
        ImGui::PushID(pushID++);

        if (ImGui::Button(labels["-"].c_str()))
        {
            tmpTmpTimetable.classes[currentClassID].timetableLessonRules.erase(
                tmpTmpTimetable.classes[currentClassID].timetableLessonRules.begin() + i);
            i--;
            ImGui::PopID();
            continue;
        }
        ImGui::SameLine();

        if (ImGui::Button(labels["Edit"].c_str()))
        {
            newRule = false;
            currentRuleID = i;
            ResetRulesVariables();
            isRules = true;
        }
        ImGui::SameLine();

        TimetableLessonRule& timetableLessonRule =
            tmpTmpTimetable.classes[currentClassID].timetableLessonRules[i];
        std::string timetableLessonRuleName = "";
        for (size_t j = 0; j < timetableLessonRule.timetableLessonIDs.size(); j++)
        {
            int timetableLessonID = timetableLessonRule.timetableLessonIDs[j];
            TimetableLesson& timetableLesson =
                tmpTmpTimetable.classes[currentClassID].timetableLessons[timetableLessonID];
            for (size_t k = 0; k < timetableLesson.lessonTeacherPairs.size(); k++)
            {
                LessonTeacherPair& lessonTeacherPair = timetableLesson.lessonTeacherPairs[k];
                timetableLessonRuleName +=
                    tmpLessons[lessonTeacherPair.lessonID].name + " (" +
                    currentTimetable.teachers[lessonTeacherPair.teacherID].name + ")";
                if (k < timetableLesson.lessonTeacherPairs.size() - 1)
                    timetableLessonRuleName += '\n';
            }
            if (j < timetableLessonRule.timetableLessonIDs.size() - 1)
                timetableLessonRuleName += "\n\n";
        }
        ImGui::Text("%s", timetableLessonRuleName.c_str());
        ImGui::NextColumn();

        ImGui::Text("%s",
                    (timetableLessonRule.preserveOrder ? labels["Yes"] : labels["No"]).c_str());
        ImGui::NextColumn();

        ImGui::Text("%d", timetableLessonRule.amount);
        ImGui::NextColumn();
        ImGui::Separator();
        ImGui::PopID();
    }
    ImGui::Columns(1);

    // Ok and Cancel
    if (ImGui::Button(labels["Ok"].c_str()))
    {
        LogInfo("Clicked the Ok button while editing class with ID " +
                std::to_string(currentClassID));
        LoadTimetableLessonsFromSelection();
        if (bulkEditClass)
        {
            ChangeClassesAmount(&tmpTmpTimetable, tmpTmpTimetable.classes[currentClassID].number,
                                bulkClassesAmount);
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
            UpdateClassLetters(&tmpTmpTimetable);
        }
        else
        {
            if (classTeacherIndex >= 0 && (size_t)classTeacherIndex < classTeacherIDs.size())
            {
                tmpTmpTimetable.classes[currentClassID].teacherID =
                    classTeacherIDs[classTeacherIndex];
            }
            else
            {
                tmpTmpTimetable.classes[currentClassID].teacherID = -1;
            }
        }
        tmpTimetable.classes = tmpTmpTimetable.classes;
        tmpTimetable.maxClassID = tmpTmpTimetable.maxClassID;
        tmpTimetable.orderedClasses = tmpTmpTimetable.orderedClasses;
        tmpLessons = tmpTmpLessons;
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

    ImGui::TextColored(
        ImVec4(255, 255, 0, 255), "%s",
        labels["Warning: changing the current year can be quite destructive."].c_str());
    ImGui::TextColored(
        ImVec4(255, 255, 0, 255), "%s",
        labels["If something went wrong, press the Cancel button to revert all changes"].c_str());
    if (ImGui::Button(labels["Back"].c_str())) ShiftClasses(&tmpTimetable, -1);
    ImGui::SameLine();
    ImGui::Text("%s", std::to_string(tmpTimetable.year).c_str());
    ImGui::SameLine();
    if (ImGui::Button(labels["Next"].c_str())) ShiftClasses(&tmpTimetable, 1);
    ImGui::Separator();

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
        FetchClassLessonsFromSimularClasses(&tmpTmpTimetable, currentClassID);
        isEditClass = true;
    }
    ImGui::Separator();

    ImGui::Columns(2);
    std::string lastClassNumber = "";
    int buttonID = 0;
    for (size_t i = 0; i < tmpTimetable.orderedClasses.size(); i++)
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
                        tmpTimetable.orderedClasses.erase(find(tmpTimetable.orderedClasses.begin(),
                                                               tmpTimetable.orderedClasses.end(),
                                                               it->first));
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
                for (size_t j = 0; j < tmpTimetable.orderedClasses.size(); j++)
                {
                    if (tmpTimetable.classes[tmpTimetable.orderedClasses[j]].number ==
                        lastClassNumber)
                        currentClassID = j;
                }
                currentClassID++;
                tmpTmpTimetable.classes = tmpTimetable.classes;
                tmpTmpTimetable.maxClassID = tmpTimetable.maxClassID;
                tmpTmpTimetable.orderedClasses = tmpTimetable.orderedClasses;
                tmpTmpTimetable.maxClassID++;
                tmpTmpTimetable.orderedClasses.insert(tmpTmpTimetable.orderedClasses.begin() +
                                                          currentClassID,
                                                      tmpTmpTimetable.maxClassID);
                tmpTmpTimetable.classes[tmpTmpTimetable.maxClassID] = Class();
                tmpTmpTimetable.classes[tmpTmpTimetable.maxClassID].number = lastClassNumber;
                tmpTmpTimetable.classes[tmpTmpTimetable.maxClassID].letter =
                    GetNthUtf8Character(labels["abcdefghijklmnopqrstuvwxyz"], currentClassID);
                currentClassID = tmpTmpTimetable.maxClassID;
                ResetVariables();
                FetchClassLessonsFromSimularClasses(&tmpTmpTimetable, tmpTmpTimetable.maxClassID);
                LogInfo("Adding a new class with number " + lastClassNumber + " and ID " +
                        std::to_string(currentClassID));
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

        ImGui::Text("%s", (tmpTimetable.classes[tmpTimetable.orderedClasses[i]].number +
                           tmpTimetable.classes[tmpTimetable.orderedClasses[i]].letter)
                              .c_str());
        ImGui::PopID();
        buttonID++;
        ImGui::Unindent();
        ImGui::NextColumn();

        if (currentTimetable.teachers.find(
                tmpTimetable.classes[tmpTimetable.orderedClasses[i]].teacherID) !=
            currentTimetable.teachers.end())
            ImGui::LabelText(
                "", "%s",
                currentTimetable
                    .teachers[tmpTimetable.classes[tmpTimetable.orderedClasses[i]].teacherID]
                    .name.c_str());
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
        currentTimetable.lessons = tmpLessons;
        currentTimetable.year = tmpTimetable.year;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button(labels["Cancel"].c_str())) *isOpen = false;
    ImGui::End();
}

#include "UI.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"

int currentClassID = 0;
bool newClass = false;
bool bulkEditClass = false;
int bulkClassesAmount = 1;
int classTeacherIndex = 0;
std::string classTeacherValues = "";
bool allClassLessons = true;
std::unordered_map<std::string, bool> classLessons;
std::unordered_map<int, int> classLessonAmounts;
std::unordered_map<int, bool> allClassLessonTeachers;
std::unordered_map<std::string, bool> classLessonTeachers;
bool allAvailableClassLessonsVertical[7];
std::vector<bool> allAvailableClassLessonsHorizontal;
std::unordered_map<int, bool> availableClassLessons;

static void ResetVariables()
{
    for (int i = 0; i < 7; i++)
        allAvailableClassLessonsVertical[i] = true;
    allAvailableClassLessonsHorizontal.clear();
    for (int i = 0; i < lessonsPerDay; i++)
        allAvailableClassLessonsHorizontal.push_back(true);

    availableClassLessons.clear();
    for (int i = 0; i < 7; i++)
    {
        for (int j = 0; j < lessonsPerDay; j++)
            availableClassLessons[i*lessonsPerDay+j] = newClass;
    }
    for (int i = 0; i < 7; i++)
    {
        for (int j = 0; j < tmpTmpTimetable.classes[currentClassID].days[i].lessonNumbers.size(); j++)
            availableClassLessons[i*lessonsPerDay + tmpTmpTimetable.classes[currentClassID].days[i].lessonNumbers[j]] = true;
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
        classLessonAmounts[lesson.first] = 1;
        for (auto& teacher: currentTimetable.teachers)
        {
            classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"] = false;
            classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"] = newClass;
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
            classLessonAmounts[lesson.first] = tmpTmpTimetable.classes[currentClassID].timetableLessons[lessonTeacherPairID].amount;
            classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"] = true;
        }
    }
}

static int currentLessonID = 0;
bool newCombinedLesson = false;
bool isCombineLessons = false;
void ShowCombineLessons(bool* isOpen)
{
    if (!ImGui::Begin("Combine lessons", isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::Columns(2);
    for (auto& lesson: currentTimetable.lessons)
    {
        if (!classLessons[std::to_string(lesson.first) + "0"]) continue;
        ImGui::PushID(lesson.first*(tmpTmpTimetable.maxTeacherID+1));
        ImGui::Checkbox(lesson.second.name.c_str(), &classLessons[std::to_string(lesson.first) + "2"]);
        ImGui::NextColumn();
        ImGui::PopID();
        for (auto& teacher: currentTimetable.teachers)
        {
            if (!classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"]) continue;
            ImGui::PushID(lesson.first*(tmpTmpTimetable.maxTeacherID+1)+teacher.first+1);
            ImGui::Checkbox(teacher.second.name.c_str(), &classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "2"]);
            ImGui::PopID();
        }
        ImGui::NextColumn();
        ImGui::Separator();
    }
    ImGui::Columns(1);
    if (ImGui::Button("Ok"))
    {
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
    if (ImGui::Button("Cancel"))
    {
        if (newCombinedLesson) tmpTmpTimetable.classes[currentClassID].timetableLessons.erase(currentLessonID);
        *isOpen = false;
    }
    ImGui::End();
}

bool isEditClass = false;
void ShowEditClass(bool* isOpen)
{
    if (!ImGui::Begin(((newClass ? "New" : "Edit") + std::string(" Class")).c_str(), isOpen))
    {
        ImGui::End();
        return;
    }
    if (bulkEditClass && !newClass)
        ImGui::TextColored(ImVec4(255, 255, 0, 255), "Warning: you are bulk editing classes!\nAfter pressing Ok ALL classes with the number below\nwill be OVERWRITTEN with the data you enter.\nIf you don't want that to happen, press the Cancel button.");
    if (ImGui::InputText("number", &tmpTmpTimetable.classes[currentClassID].number))
        ResetVariables();
    if (bulkEditClass)
    {
        ImGui::InputInt("amount", &bulkClassesAmount);
        if (bulkClassesAmount < 1) bulkClassesAmount = 1;
    }
    else
    {
        ImGui::InputText("letter", &tmpTmpTimetable.classes[currentClassID].letter);
        ImGui::Combo("teacher", &classTeacherIndex, classTeacherValues.c_str());
    }
    ImGui::Separator();
    ImGui::Text("available-lessons");
    ImGui::Separator();
    ImGui::Columns(8);
    ImGui::LabelText("##1", "");
    ImGui::LabelText("##2", "");
    for (int i = 0; i < lessonsPerDay; i++)
    {
        ImGui::PushID(i+3);
        bool availableClassLessonsHorizontal = allAvailableClassLessonsHorizontal[i];
        if (ImGui::Checkbox(std::to_string(i).c_str(), &availableClassLessonsHorizontal))
        {
            allAvailableClassLessonsHorizontal[i] = availableClassLessonsHorizontal;
            for (int j = 0; j < 7; j++)
            {
                for (int k = 0; k < tmpTmpTimetable.classes[currentClassID].days[j].lessonNumbers.size(); k++)
                {
                    if (tmpTmpTimetable.classes[currentClassID].days[j].lessonNumbers[k] == i)
                    {
                        tmpTmpTimetable.classes[currentClassID].days[j].lessonNumbers.erase(
                            tmpTmpTimetable.classes[currentClassID].days[j].lessonNumbers.begin() + k);
                        availableClassLessons[j*lessonsPerDay+i] = availableClassLessonsHorizontal;
                        k--;
                    }
                }
                if (availableClassLessonsHorizontal)
                {
                    tmpTmpTimetable.classes[currentClassID].days[j].lessonNumbers.push_back(i);
                    availableClassLessons[j*lessonsPerDay+i] = availableClassLessonsHorizontal;
                }
            }
        }
        ImGui::PopID();
    }
    ImGui::NextColumn();
    for (int i = 0; i < 7; i++)
    {
        ImGui::Text(weekDays[i].c_str());
        ImGui::PushID(i*(lessonsPerDay+1)+7+tmpTmpTimetable.lessons.size());
        if (ImGui::Checkbox((allAvailableClassLessonsVertical[i] ? "Deselect all" : "Select all"), &allAvailableClassLessonsVertical[i]))
        {
            for (int j = 0; j < lessonsPerDay; j++)
                availableClassLessons[i*lessonsPerDay+j] = allAvailableClassLessonsVertical[i];
        }
        ImGui::PopID();
        for (int j = 0; j < lessonsPerDay; j++)
        {
            ImGui::PushID(i*(lessonsPerDay+1)+j+tmpTmpTimetable.lessons.size());
            ImGui::Checkbox("", &availableClassLessons[i*lessonsPerDay+j]);
            ImGui::PopID();
        }
        ImGui::NextColumn();
    }
    ImGui::Columns(1);
    ImGui::Separator();
    ImGui::LabelText("", "lessons");
    ImGui::Separator();
    if (ImGui::Button("Combine lessons"))
    {
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
            it++;
            continue;
        }
        ImGui::PushID(it->first);
        if (ImGui::Button("-"))
        {
            ImGui::PopID();
            it = tmpTmpTimetable.classes[currentClassID].timetableLessons.erase(it);
            break;
        }
        ImGui::SameLine();
        if (ImGui::Button("Edit"))
        {
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
        it++;
    }
    ImGui::Separator();
    ImGui::Columns(2);
    if (ImGui::Checkbox((allClassLessons ? "Deselect all##1" : "Select all##1"), &allClassLessons))
    {
        for (auto& lesson: currentTimetable.lessons)
            classLessons[std::to_string(lesson.first) + "1"] = allClassLessons;
    }
    for (auto& lesson: currentTimetable.lessons)
    {
        if (!classLessons[std::to_string(lesson.first) + "0"]) continue;
        ImGui::PushID(lesson.first*(currentTimetable.teachers.size()+1));
        ImGui::InputInt(lesson.second.name.c_str(), &classLessonAmounts[lesson.first]);
        ImGui::NextColumn();
        if (ImGui::Checkbox((allClassLessonTeachers[lesson.first] ? "Deselect all##1" : "Select all##1"),
        &allClassLessonTeachers[lesson.first]))
        {
            for (auto& teacher: currentTimetable.teachers)
                classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"] =
                allClassLessonTeachers[lesson.first];
        }
        ImGui::PopID();
        for (auto& teacher: currentTimetable.teachers)
        {
            if (!classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"]) continue;
            ImGui::PushID(lesson.first*(currentTimetable.teachers.size()+1)+teacher.first+1);
            ImGui::Checkbox(teacher.second.name.c_str(),
                &classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"]);
            ImGui::PopID();
        }
        ImGui::NextColumn();
        ImGui::Separator();
    }
    ImGui::Columns(1);
    if (ImGui::Button("Ok"))
    {
        for (int i = 0; i < 7; i++)
        {
            tmpTmpTimetable.classes[currentClassID].days[i].lessonNumbers.clear();
            for (int j = 0; j < lessonsPerDay; j++)
            {
                if (availableClassLessons[i*lessonsPerDay+j])
                    tmpTmpTimetable.classes[currentClassID].days[i].lessonNumbers.push_back(j);
            }
        }
        for (auto it = tmpTmpTimetable.classes[currentClassID].timetableLessons.begin(); it != tmpTmpTimetable.classes[currentClassID].timetableLessons.end();)
        {
            if (it->second.lessonTeacherPairs.size() <= 1)
            {
                it = tmpTmpTimetable.classes[currentClassID].timetableLessons.erase(it);
                continue;
            }
            it++;
        }
        for (auto& lesson: currentTimetable.lessons)
        {
            if (classLessonAmounts[lesson.first] == 0) continue;
            for (auto& teacher: currentTimetable.teachers)
            {
                if (!classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"]) continue;
                tmpTmpTimetable.classes[currentClassID].maxTimetableLessonID++;
                tmpTmpTimetable.classes[currentClassID].timetableLessons[tmpTmpTimetable.classes[currentClassID].maxTimetableLessonID] =
                    TimetableLesson();
                tmpTmpTimetable.classes[currentClassID].timetableLessons[tmpTmpTimetable.classes[currentClassID].maxTimetableLessonID].amount =
                    classLessonAmounts[lesson.first];
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
            if (!newClass)
            {
                for (auto it = tmpTmpTimetable.classes.begin(); it != tmpTmpTimetable.classes.end();)
                {
                    if (it->second.number == tmpTmpTimetable.classes[currentClassID].number && it->first != currentClassID)
                    {
                        tmpTmpTimetable.orderedClasses.erase(
                            find(tmpTmpTimetable.orderedClasses.begin(), tmpTmpTimetable.orderedClasses.end(), it->first));
                        it = tmpTmpTimetable.classes.erase(it);
                        continue;
                    }
                    it++;
                }
            }
            int orderedClassesID = -1;
            for (int i = 0; i < tmpTmpTimetable.orderedClasses.size(); i++)
            {
                if (tmpTmpTimetable.orderedClasses[i] == currentClassID)
                {
                    orderedClassesID = i;
                    break;
                }
            }
            for (int i = 0; i < bulkClassesAmount-1; i++)
            {
                tmpTmpTimetable.maxClassID++;
                tmpTmpTimetable.orderedClasses.insert(tmpTmpTimetable.orderedClasses.begin() + orderedClassesID + 1, tmpTmpTimetable.maxClassID);
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
                tmpTmpTimetable.classes[tmpTmpTimetable.orderedClasses[orderedClassesID + i]].letter = 'a' + i;
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
    if (ImGui::Button("Cancel")) *isOpen = false;
    ImGui::End();
}

bool isClasses = false;
void ShowClasses(bool* isOpen)
{
    if (!ImGui::Begin("Classes", isOpen))
    {
        ImGui::End();
        return;
    }
    if (ImGui::Button("+"))
    {
        newClass = true;
        bulkEditClass = true;
        tmpTmpTimetable.classes = tmpTimetable.classes;
        tmpTmpTimetable.maxClassID = tmpTimetable.maxClassID;
        tmpTmpTimetable.orderedClasses = tmpTimetable.orderedClasses;
        tmpTmpTimetable.maxClassID++;
        currentClassID = tmpTmpTimetable.maxClassID;
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
            if (ImGui::Button("-"))
            {
                ImGui::PopID();
                for (auto it2 = tmpTimetable.classes.begin(); it2 != tmpTimetable.classes.end();)
                {
                    if (it2->second.number == lastClassNumber)
                    {
                        tmpTimetable.orderedClasses.erase(find(tmpTimetable.orderedClasses.begin(), tmpTimetable.orderedClasses.end(), it2->first));
                        it2 = tmpTimetable.classes.erase(it2);
                        continue;
                    }
                    it2++;
                }
                break;
            }
            ImGui::SameLine();
            if (ImGui::Button("Edit"))
            {
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
            ImGui::Text(lastClassNumber.c_str());
            ImGui::Indent();
            if (ImGui::Button("+"))
            {
                std::cout << classTeacherValues << "\n";
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
                ResetVariables();
                isEditClass = true;
            }
            ImGui::Unindent();
            ImGui::NextColumn();
            ImGui::LabelText("", "");
            ImGui::NextColumn();
            ImGui::PopID();
            buttonID++;
        }
        ImGui::Indent();
        ImGui::PushID(buttonID);
        if (ImGui::Button("-"))
        {
            tmpTimetable.classes.erase(tmpTimetable.orderedClasses[i]);
            tmpTimetable.orderedClasses.erase(tmpTimetable.orderedClasses.begin() + i);
            i--;
        }
        ImGui::SameLine();
        if (ImGui::Button("Edit"))
        {
            newClass = false;
            bulkEditClass = false;
            tmpTmpTimetable.classes = tmpTimetable.classes;
            tmpTmpTimetable.maxClassID = tmpTimetable.maxClassID;
            tmpTmpTimetable.orderedClasses = tmpTimetable.orderedClasses;
            currentClassID = tmpTmpTimetable.orderedClasses[i];
            ResetVariables();
            isEditClass = true;
        }
        ImGui::SameLine();
        ImGui::Text((tmpTimetable.classes[tmpTimetable.orderedClasses[i]].number + tmpTimetable.classes[tmpTimetable.orderedClasses[i]].letter).c_str());
        ImGui::PopID();
        buttonID++;
        ImGui::Unindent();
        ImGui::NextColumn();
        if (currentTimetable.teachers.find(tmpTimetable.classes[tmpTimetable.orderedClasses[i]].teacherID) !=
        currentTimetable.teachers.end())
            ImGui::LabelText("", currentTimetable.teachers[tmpTimetable.classes[tmpTimetable.orderedClasses[i]].teacherID].name.c_str());
        ImGui::NextColumn();
    }
    ImGui::Columns(1);
    ImGui::Separator();
    if (ImGui::Button("Ok"))
    {
        currentTimetable.classes = tmpTimetable.classes;
        currentTimetable.maxClassID = tmpTimetable.maxClassID;
        currentTimetable.orderedClasses = tmpTimetable.orderedClasses;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) *isOpen = false;
    ImGui::End();
}

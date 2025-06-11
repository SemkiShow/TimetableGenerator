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
        for (int j = 0; j < tmpTimetable.classes[currentClassID].days[i].lessonNumbers.size(); j++)
            availableClassLessons[i*lessonsPerDay + tmpTimetable.classes[currentClassID].days[i].lessonNumbers[j]] = true;
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
        classLessons[std::to_string(lesson.first) + "1"] = false;
        for (auto& teacher: currentTimetable.teachers)
        {
            classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"] = false;
            classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"] = false;
        }
    }

    for (auto& lesson: currentTimetable.lessons)
    {
        bool classIDFound = false;
        for (int i = 0; i < lesson.second.classIDs.size(); i++)
        {
            if (currentClassID == lesson.second.classIDs[i] ||
                (tmpTimetable.classes[currentClassID].number == tmpTimetable.classes[lesson.second.classIDs[i]].number
                    && bulkEditClass))
            {
                classIDFound = true;
                break;
            }
        }
        if (!classIDFound) continue;
        classLessons[std::to_string(lesson.first) + "0"] = true;
        classLessons[std::to_string(lesson.first) + "1"] = true;
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
            classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"] = true;
        }
    }
}

static int currentLessonIndex = 0;
bool isCombineLessons = false;
void ShowCombineLessons(bool* isOpen)
{
    
}

bool isEditClass = false;
void ShowEditClass(bool* isOpen)
{

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
        tmpTimetable.maxClassID++;
        currentClassID = tmpTimetable.maxClassID;
        tmpTimetable.orderedClasses.push_back(currentClassID);
        tmpTimetable.classes[currentClassID] = Class();
        tmpTimetable.classes[currentClassID].number = "0";
        ResetVariables();
        tmpTmpTimetable.classes = tmpTimetable.classes;
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
                std::cout << "Removed a Class() group!\n";
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
                currentClassID = i;
                std::cout << "Bulk edited class!\n";
                ResetVariables();
                tmpTmpTimetable.classes = tmpTimetable.classes;
                isEditClass = true;
            }
            ImGui::SameLine();
            ImGui::Text(lastClassNumber.c_str());
            ImGui::Indent();
            if (ImGui::Button("+"))
            {
                ImGui::PopID();
                newClass = true;
                bulkEditClass = false;
                for (int j = 0; j < tmpTimetable.orderedClasses.size(); j++)
                {
                    if (tmpTimetable.classes[tmpTimetable.orderedClasses[j]].number == lastClassNumber) currentClassID = j;
                }
                currentClassID++;
                tmpTimetable.maxClassID++;
                tmpTimetable.orderedClasses.insert(tmpTimetable.orderedClasses.begin() + currentClassID, tmpTimetable.maxClassID);
                tmpTimetable.classes[tmpTimetable.maxClassID] = Class();
                tmpTimetable.classes[tmpTimetable.maxClassID].number = tmpTimetable.classes[tmpTimetable.orderedClasses[currentClassID-1]].number;
                currentClassID = tmpTimetable.maxClassID;
                std::cout << "Inserted a Class()!\n";
                ResetVariables();
                tmpTmpTimetable.classes = tmpTimetable.classes;
                isEditClass = true;
                break;
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
            ImGui::PopID();
            tmpTimetable.classes.erase(tmpTimetable.orderedClasses[i]);
            tmpTimetable.orderedClasses.erase(tmpTimetable.orderedClasses.begin() + i);
            i--;
            std::cout << "Removed a Class()!\n";
            break;
        }
        ImGui::SameLine();
        if (ImGui::Button("Edit"))
        {
            newClass = false;
            bulkEditClass = false;
            currentClassID = i;
            std::cout << "Edited class!\n";
            ResetVariables();
            tmpTmpTimetable.classes = tmpTimetable.classes;
            isEditClass = true;
        }
        ImGui::SameLine();
        ImGui::Text((tmpTimetable.classes[tmpTimetable.orderedClasses[i]].number + tmpTimetable.classes[tmpTimetable.orderedClasses[i]].letter).c_str());
        ImGui::PopID();
        buttonID++;
        ImGui::Unindent();
        ImGui::NextColumn();
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

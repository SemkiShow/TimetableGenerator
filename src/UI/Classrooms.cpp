#include "UI.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"

bool isClassrooms = false;
std::string classrooms = "";
void ShowClassrooms(bool* isOpen)
{
    if (!ImGui::Begin("Classrooms", isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::InputTextMultiline("##", &classrooms);
    if (ImGui::Button("Ok"))
    {
        currentTimetable.classrooms.clear();
        std::vector<std::string> classroomsVector = Split(classrooms, '\n');
        for (int i = 0; i < classroomsVector.size(); i++)
        {
            if (classroomsVector[i] == "") continue;
            currentTimetable.classrooms[i] = Classroom();
            currentTimetable.classrooms[i].name = classroomsVector[i];
        }
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) *isOpen = false;
    ImGui::End();
}

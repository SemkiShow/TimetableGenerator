#include "Searching.hpp"
#include "Settings.hpp"

int GetLessonsAmount(std::map<int, TimetableLesson> timetableLessons)
{
    int output = 0;
    for (auto& lesson: timetableLessons)
        output += lesson.second.amount;
    return output;
}

int GetLessonPlacesAmount(Day days[DAYS_PER_WEEK])
{
    int output = 0;
    for (int i = 0; i < DAYS_PER_WEEK; i++)
    {
        for (int j = 0; j < days[i].lessons.size(); j++)
        {
            if (days[i].lessons[j]) output++;
        }
    }
    return output;
}

bool IsTimetableCorrect(Timetable* timetable)
{
    for (auto& classPair: timetable->classes)
    {
        if (GetLessonsAmount(classPair.second.timetableLessons) >
        GetLessonPlacesAmount(classPair.second.days))
            return false;
    }
    return true;
}

void RandomizeTimetable(Timetable* timetable)
{
    if (!IsTimetableCorrect(timetable))
    {
        std::cerr << "Error: the timetable is incorrect!\n";
        return;
    }
    for (auto& classPair: timetable->classes)
    {
        std::vector<int> timetableLessonIDs;
        int counter = 0;
        for (auto& lesson: classPair.second.timetableLessons)
        {
            for (int i = 0; i < lesson.second.amount; i++)
                timetableLessonIDs.push_back(lesson.first);
        }
        auto rng = std::default_random_engine(time(0));
        std::shuffle(timetableLessonIDs.begin(), timetableLessonIDs.end(), rng);
        counter = 0;
        for (int i = 0; i < DAYS_PER_WEEK; i++)
        {
            classPair.second.days[i].timetableLessonIDs.clear();
            for (int j = 0; j < classPair.second.days[i].lessons.size(); j++)
            {
                if (classPair.second.days[i].lessons[j])
                {
                    classPair.second.days[i].timetableLessonIDs.push_back(timetableLessonIDs[counter]);
                    counter++;
                }
                else classPair.second.days[i].timetableLessonIDs.push_back(-3);
            }
        }
    }
}

#pragma once

#include "Timetable.hpp"
#include <string>
#include <vector>

extern int currentClassID;
extern bool newClass;
extern int classTeacherIndex;
extern std::string classTeacherValues;
extern std::vector<int> classTeacherIDs;
extern std::map<int, Lesson> tmpTmpLessons;

void ResetClassTeacherValues();
std::string GetNthUtf8Character(const std::string& utf8String, int index);
bool CompareTimetableLessons(const TimetableLesson lesson1, const TimetableLesson lesson2);
void FetchClassLessonsFromSimularClasses(Timetable* timetable, int classID);
void ChangeClassesAmount(Timetable* timetable, const std::string& classNumber,
                         const int classesAmount);
void UpdateClassLetters(Timetable* timetable);
int GetClassesAmount(Timetable* timetable, const std::string& classNumber);
void ShiftClasses(Timetable* timetable, const int direction);

// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Logging.hpp"
#include "Searching.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"
#include "UI.hpp"
#include <algorithm>
#include <curl/curl.h>
#include <iostream>
#include <random>
#include <string>
#include <thread>

static thread_local std::mt19937 rng(std::random_device{}());
unsigned int threadsNumber = std::max(std::thread::hardware_concurrency(), (unsigned int)1);
IterationData iterationData = IterationData();

int GetLessonsAmount(const std::map<int, TimetableLesson> timetableLessons)
{
    int output = 0;
    for (auto& lesson: timetableLessons)
        output += lesson.second.amount;
    return output;
}

int GetLessonPlacesAmount(const std::vector<Day> days)
{
    int output = 0;
    for (size_t i = 0; i < iterationData.daysPerWeek; i++)
    {
        for (size_t j = 0; j < days[i].lessons.size(); j++)
        {
            if (days[i].lessons[j]) output++;
        }
    }
    return output;
}

bool IsTimetableCorrect(const Timetable& timetable)
{
    for (auto& classPair: timetable.classes)
    {
        if (GetLessonsAmount(classPair.second.timetableLessons) >
            GetLessonPlacesAmount(classPair.second.days))
            return false;
    }
    return true;
}

// Shuffle assign timetable lessons to classes
void RandomizeTimetable(Timetable& timetable)
{
    if (!IsTimetableCorrect(timetable))
    {
        std::cerr << "Error: the timetable is incorrect!\n";
        return;
    }
    for (auto& classPair: timetable.classes)
    {
        std::vector<int> timetableLessonIDs;
        for (auto& lesson: classPair.second.timetableLessons)
        {
            for (int i = 0; i < lesson.second.amount; i++)
                timetableLessonIDs.push_back(lesson.first);
        }
        std::shuffle(timetableLessonIDs.begin(), timetableLessonIDs.end(), rng);
        size_t counter = 0;
        classPair.second.days.resize(iterationData.daysPerWeek);
        for (size_t i = 0; i < iterationData.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.clear();
            for (size_t j = 0; j < classPair.second.days[i].lessons.size(); j++)
            {
                classPair.second.days[i].classroomLessonPairs.push_back(ClassroomLessonPair());
                if (classPair.second.days[i].lessons[j])
                {
                    if (counter < timetableLessonIDs.size())
                    {
                        classPair.second.days[i].classroomLessonPairs[j].timetableLessonID =
                            timetableLessonIDs[counter];
                        for (size_t k = 0;
                             k < classPair.second.timetableLessons[timetableLessonIDs[counter]]
                                     .lessonTeacherPairs.size();
                             k++)
                        {
                            int lessonID =
                                classPair.second.timetableLessons[timetableLessonIDs[counter]]
                                    .lessonTeacherPairs[k]
                                    .lessonID;
                            int classroomID =
                                timetable.lessons[lessonID]
                                    .classroomIDs[rand() %
                                                  timetable.lessons[lessonID].classroomIDs.size()];
                            classPair.second.days[i].classroomLessonPairs[j].classroomIDs.push_back(
                                classroomID);
                        }
                        counter++;
                    }
                    else
                        classPair.second.days[i].classroomLessonPairs[j].timetableLessonID =
                            ANY_LESSON;
                }
                else
                    classPair.second.days[i].classroomLessonPairs[j].timetableLessonID = NO_LESSON;
            }
        }
    }
}

void SwapRandomTimetableLessons(Timetable& timetable)
{
    std::uniform_int_distribution<int> classDistribution(0, timetable.orderedClasses.size() - 1);
    std::uniform_int_distribution<int> dayDistribution(0, iterationData.daysPerWeek - 1);
    std::uniform_int_distribution<int> lesson1Distribution;
    std::uniform_int_distribution<int> lesson2Distribution;
    int classID, classIndex, lesson1Day, lesson2Day, lesson1Index, lesson2Index;
    while (true)
    {
        // Get class ID
        if (timetable.orderedClasses.empty()) std::cerr << "The timetable's classes are empty!\n";
        classIndex = classDistribution(rng);
        classID = timetable.orderedClasses[classIndex];

        // Get day ID
        lesson1Day = dayDistribution(rng);
        lesson2Day = dayDistribution(rng);

        // Get ClassroomLessonPair ID
        if (timetable.classes[classID].days[lesson1Day].classroomLessonPairs.empty() ||
            timetable.classes[classID].days[lesson2Day].classroomLessonPairs.empty())
            continue;
        lesson1Distribution = std::uniform_int_distribution<int>(
            0, timetable.classes[classID].days[lesson1Day].classroomLessonPairs.size() - 1);
        lesson2Distribution = std::uniform_int_distribution<int>(
            0, timetable.classes[classID].days[lesson2Day].classroomLessonPairs.size() - 1);
        lesson1Index = lesson1Distribution(rng);
        lesson2Index = lesson2Distribution(rng);

        // Exit if found a valid lesson
        if (timetable.classes[classID]
                    .days[lesson1Day]
                    .classroomLessonPairs[lesson1Index]
                    .timetableLessonID >= ANY_LESSON &&
            timetable.classes[classID]
                    .days[lesson2Day]
                    .classroomLessonPairs[lesson2Index]
                    .timetableLessonID >= ANY_LESSON)
            break;
    }
    // Swap 2 timetable lessons in the same class
    ClassroomLessonPair buf =
        timetable.classes[classID].days[lesson2Day].classroomLessonPairs[lesson2Index];
    timetable.classes[classID].days[lesson2Day].classroomLessonPairs[lesson2Index] =
        timetable.classes[classID].days[lesson1Day].classroomLessonPairs[lesson1Index];
    timetable.classes[classID].days[lesson1Day].classroomLessonPairs[lesson1Index] = buf;
}

double EvaluateFitness(const Timetable& timetable)
{
    return timetable.bonusPoints - (timetable.errors * errorBonusRatio);
}

Timetable TournamentSelection(const Timetable* population)
{
    size_t tournamentSize = 7;
    auto populationDistribution =
        std::uniform_int_distribution<int>(0, iterationData.timetablesPerGeneration - 1);
    int bestID = populationDistribution(rng);

    for (size_t i = 0; i < tournamentSize; i++)
    {
        int challengerID = populationDistribution(rng);
        if (EvaluateFitness(population[challengerID]) > EvaluateFitness(population[bestID]))
            bestID = challengerID;
    }

    return population[bestID];
}

Timetable Crossover(const Timetable& parent1, const Timetable& parent2)
{
    Timetable child = parent1;
    auto distribution2 = std::uniform_int_distribution<int>(0, 1);

    for (auto& parent2Classes: parent2.classes)
    {
        if (distribution2(rng) == 0) child.classes[parent2Classes.first] = parent2Classes.second;
    }

    return child;
}

void MutateTimetableClassroom(Timetable& timetable)
{
    std::uniform_int_distribution<int> classDistribution(0, timetable.orderedClasses.size() - 1);
    std::uniform_int_distribution<int> dayDistribution(0, iterationData.daysPerWeek - 1);
    std::uniform_int_distribution<int> lessonDistribution;
    std::uniform_int_distribution<int> classroomDistribution;
    std::uniform_int_distribution<int> lessonTeacherPairDistribution;
    while (true)
    {
        // Get class ID
        if (timetable.orderedClasses.empty()) std::cerr << "The timetable's classes are empty!\n";
        int classIndex = classDistribution(rng);
        int classID = timetable.orderedClasses[classIndex];

        // Get day ID
        int dayID = dayDistribution(rng);

        // Get ClassroomLessonPair ID
        if (timetable.classes[classID].days[dayID].classroomLessonPairs.empty()) continue;
        lessonDistribution = std::uniform_int_distribution<int>(
            0, timetable.classes[classID].days[dayID].classroomLessonPairs.size() - 1);
        int classroomLessonPairID = lessonDistribution(rng);

        // Get classroom ID
        if (timetable.classes[classID]
                .days[dayID]
                .classroomLessonPairs[classroomLessonPairID]
                .classroomIDs.empty())
            continue;
        classroomDistribution = std::uniform_int_distribution<int>(
            0, timetable.classes[classID]
                       .days[dayID]
                       .classroomLessonPairs[classroomLessonPairID]
                       .classroomIDs.size() -
                   1);
        int classroomID = classroomDistribution(rng);

        // Get timetable lesson ID
        if (timetable.classes[classID]
                .days[dayID]
                .classroomLessonPairs[classroomLessonPairID]
                .timetableLessonID < 0)
            continue;
        int timetableLessonID = timetable.classes[classID]
                                    .days[dayID]
                                    .classroomLessonPairs[classroomLessonPairID]
                                    .timetableLessonID;
        if (timetableLessonID < 0) continue;

        // Get LessonTeacherPair ID
        if (timetable.classes[classID]
                .timetableLessons[timetableLessonID]
                .lessonTeacherPairs.empty())
            continue;
        lessonTeacherPairDistribution =
            std::uniform_int_distribution<int>(0, timetable.classes[classID]
                                                          .timetableLessons[timetableLessonID]
                                                          .lessonTeacherPairs.size() -
                                                      1);
        int lessonTeacherPairID = lessonTeacherPairDistribution(rng);

        // Generate a lessonID
        int lessonID = timetable.classes[classID]
                           .timetableLessons[timetableLessonID]
                           .lessonTeacherPairs[lessonTeacherPairID]
                           .lessonID;
        if (timetable.lessons[lessonID].classroomIDs.empty()) continue;

        // Generate and replace a new classroom ID
        classroomDistribution = std::uniform_int_distribution<int>(
            0, timetable.lessons[lessonID].classroomIDs.size() - 1);
        int newClassroomID = timetable.lessons[lessonID].classroomIDs[classroomDistribution(rng)];
        timetable.classes[classID]
            .days[dayID]
            .classroomLessonPairs[classroomLessonPairID]
            .classroomIDs[classroomID] = newClassroomID;
        return;
    }
}

void MutateTimetable(Timetable& timetable)
{
    size_t swapsAmount = std::uniform_int_distribution<int>(0, 30)(rng);
    for (size_t i = 0; i < swapsAmount; i++)
    {
        SwapRandomTimetableLessons(timetable);
    }
    size_t classroomsMutationsAmount = std::uniform_int_distribution<int>(0, 10)(rng);
    for (size_t i = 0; i < classroomsMutationsAmount; i++)
    {
        MutateTimetableClassroom(timetable);
    }
}

void GeneticAlgorithm(int threadID, Timetable* population, Timetable* newPopulation)
{
    for (size_t i = threadID * iterationData.timetablesPerGeneration / threadsNumber;
         i < (threadID + 1) * iterationData.timetablesPerGeneration / threadsNumber; i++)
    {
        Timetable parent1 = TournamentSelection(population);
        Timetable parent2 = TournamentSelection(population);

        Timetable child = Crossover(parent1, parent2);
        MutateTimetable(child);
        if (verboseLogging)
        {
            std::cout << "\x1b[32mScoring timetable " << i << "\x1b[0m. ";
        }
        ScoreTimetable(child);
        newPopulation[i] = child;
    }
}

void GetBestSpecies(Timetable* timetables, Timetable* population, Timetable* newPopulation,
                    int* minErrors)
{
    double averageFitness = 0;
    for (int i = 0; i < iterationData.timetablesPerGeneration; i++)
    {
        averageFitness += EvaluateFitness(population[i]);
        averageFitness += EvaluateFitness(newPopulation[i]);
    }
    averageFitness /= iterationData.timetablesPerGeneration * 2;

    int counter = 1;

    // Selecting the new population with above average fitness and minimal errors
    for (int i = 0; i < iterationData.timetablesPerGeneration; i++)
    {
        if (counter >= iterationData.timetablesPerGeneration) break;
        if (newPopulation[i].errors < *minErrors) *minErrors = newPopulation[i].errors;
        if (EvaluateFitness(newPopulation[i]) >= averageFitness &&
            newPopulation[i].errors <= *minErrors)
            timetables[counter++] = newPopulation[i];
    }

    // Selecting the new population with above average fitness and more than minimal errors
    for (int i = 0; i < iterationData.timetablesPerGeneration; i++)
    {
        if (counter >= iterationData.timetablesPerGeneration) break;
        if (newPopulation[i].errors < *minErrors) *minErrors = newPopulation[i].errors;
        if (EvaluateFitness(newPopulation[i]) >= averageFitness &&
            newPopulation[i].errors > *minErrors)
            timetables[counter++] = newPopulation[i];
    }

    // Selecting the old population with above average fitness and minimal errors
    for (int i = 0; i < iterationData.timetablesPerGeneration; i++)
    {
        if (counter >= iterationData.timetablesPerGeneration) break;
        if (population[i].errors < *minErrors) *minErrors = population[i].errors;
        if (EvaluateFitness(population[i]) >= averageFitness && population[i].errors <= *minErrors)
            timetables[counter++] = population[i];
    }

    // Selecting the old population with above average fitness and more than minimal errors
    for (int i = 0; i < iterationData.timetablesPerGeneration; i++)
    {
        if (counter >= iterationData.timetablesPerGeneration) break;
        if (population[i].errors < *minErrors) *minErrors = population[i].errors;
        if (EvaluateFitness(population[i]) >= averageFitness && population[i].errors > *minErrors)
            timetables[counter++] = population[i];
    }

    // Choosing the rest of the population randomly from the old and new populations
    for (int i = counter; i < iterationData.timetablesPerGeneration; i++)
    {
        if (std::uniform_int_distribution<int>(0, 1)(rng) == 0)
        {
            timetables[i] = population[std::uniform_int_distribution<int>(
                0, iterationData.timetablesPerGeneration - 1)(rng)];
        }
        else
        {
            timetables[i] = newPopulation[std::uniform_int_distribution<int>(
                0, iterationData.timetablesPerGeneration - 1)(rng)];
        }
    }

    std::cout << "Selected " << iterationData.timetablesPerGeneration - counter << "/"
              << iterationData.timetablesPerGeneration << " random timetables. ";
}

int GetBestTimetableIndex(const Timetable* timetables)
{
    double bestTimetableScore = INT_MIN;
    int bestTimetableIndex = 0;
    double timetableScore = INT_MIN;
    for (int i = 0; i < iterationData.timetablesPerGeneration; i++)
    {
        timetableScore = EvaluateFitness(timetables[i]);
        if (timetables[i].errors < iterationData.minErrors)
            iterationData.minErrors = timetables[i].errors;
        if (timetables[i].errors == 0 && timetables[i].bonusPoints > iterationData.maxBonusPoints)
        {
            iterationData.maxBonusPoints = timetables[i].bonusPoints;
        }
        if (timetableScore > bestTimetableScore && timetables[i].errors <= iterationData.minErrors)
        {
            bestTimetableScore = timetableScore;
            bestTimetableIndex = i;
        }
    }
    return bestTimetableIndex;
}

void InjectRandomImmigrants(Timetable* population)
{
    for (int i = 0;
         i < std::uniform_int_distribution<int>(0, iterationData.timetablesPerGeneration / 10)(rng);
         i++)
    {
        int idx =
            std::uniform_int_distribution<int>(1, iterationData.timetablesPerGeneration - 1)(rng);
        Timetable immigrant = currentTimetable;
        RandomizeTimetable(immigrant);
        ScoreTimetable(immigrant);
        population[idx] = immigrant;
    }
}

void RunASearchIteration()
{
    // Wait for another thread
    while (iterationData.threadLock)
        COMPILER_BARRIER();

    // Make a thread lock
    iterationData.threadLock = true;

    // Change the status is a timetable with 0 errors is found
    if (iterationData.timetables[iterationData.bestTimetableIndex].errors == 0)
    {
        if (iterationData.startBonusPoints == INT_MAX)
        {
            iterationData.startBonusPoints =
                iterationData.timetables[iterationData.bestTimetableIndex].bonusPoints;
        }
        generateTimetableStatus = labels["Finding additional bonus points..."];
    }

    // Exit if there are the additional bonus points counter is over the limit or the iteratiuon
    // count is over the limit
    if (iterationData.timetables[iterationData.bestTimetableIndex].bonusPoints -
                iterationData.startBonusPoints >=
            additionalBonusPoints ||
        (maxIterations != -1 && iterationData.iteration >= maxIterations))
    {
        iterationData.isDone = true;
        iterationData.threadLock = false;
        generateTimetableStatus = labels["Timetable generating done!"];
        return;
    }

    // Init the threads
    std::thread* threads = new std::thread[threadsNumber];

    // Output debug info
    std::cout << "\x1b[34mIteration " << iterationData.iteration++ << "\x1b[0m. ";
    std::cout << "The best score is " << iterationData.allTimeBestScore << ". ";
    std::cout << "The best timetable has "
              << iterationData.timetables[iterationData.bestTimetableIndex].errors << " errors. ";
    std::cout << "The best timetable has "
              << iterationData.timetables[iterationData.bestTimetableIndex].bonusPoints
              << " bonus points. ";
    std::cout << iterationData.iterationsPerChange
              << " iterations have passed since last score improvement. ";
    if (iterationData.iteration % 10 == 0)
    {
        LogInfo("Iteration: " + std::to_string(iterationData.iteration));
        LogInfo("The best score is " + std::to_string(iterationData.allTimeBestScore));
        LogInfo("The best timetable has " +
                std::to_string(iterationData.timetables[iterationData.bestTimetableIndex].errors) +
                " errors");
        LogInfo(
            "The best timetable has " +
            std::to_string(iterationData.timetables[iterationData.bestTimetableIndex].bonusPoints) +
            " bonus points");
        LogInfo(std::to_string(iterationData.iterationsPerChange) +
                " iterations have passed since last score improvement");
    }

    // Change timetables per generation dynamically
    iterationData.timetablesPerGeneration =
        std::min(iterationData.maxTimetablesPerGeneration,
                 std::max(iterationData.minTimetablesPerGeneration,
                          (iterationData.iterationsPerChange + 1) * timetablesPerGenerationStep));

    // Inject random immigrants
    if (iterationData.iteration % 10 == 0)
    {
        InjectRandomImmigrants(iterationData.timetables);
    }

    // Run worker threads
    for (size_t i = 0; i < threadsNumber; i++)
        threads[i] =
            std::thread(GeneticAlgorithm, i, iterationData.timetables, iterationData.newPopulation);
    for (size_t i = 0; i < threadsNumber; i++)
        threads[i].join();

    // Get the best timetable from the current generation
    iterationData.bestTimetableIndex = GetBestTimetableIndex(iterationData.timetables);
    iterationData.bestScore =
        EvaluateFitness(iterationData.timetables[iterationData.bestTimetableIndex]);

    // Save the best timetable at index 0 (elitism)
    Timetable bufTimetable = iterationData.timetables[0];
    iterationData.timetables[0] = iterationData.timetables[iterationData.bestTimetableIndex];
    iterationData.timetables[iterationData.bestTimetableIndex] = bufTimetable;

    // Save only the best species from the old and new populations
    for (int i = 0; i < iterationData.timetablesPerGeneration; i++)
        iterationData.population[i] = iterationData.timetables[i];
    GetBestSpecies(iterationData.timetables, iterationData.population, iterationData.newPopulation,
                   &iterationData.minErrors);

    // Change allTimeBestScore if current best score is better
    std::cout << '\n';
    if (iterationData.bestScore > iterationData.allTimeBestScore)
        iterationData.allTimeBestScore = iterationData.bestScore;
    if (iterationData.lastAllTimeBestScore == iterationData.allTimeBestScore)
        iterationData.iterationsPerChange++;
    else
        iterationData.iterationsPerChange = 0;
    iterationData.lastAllTimeBestScore = iterationData.allTimeBestScore;
    iterationData.lastBestScore = iterationData.bestScore;

    // Update the error plot
    for (size_t i = 0; i < iterationData.errorValuesPoints - 1; i++)
    {
        iterationData.errorValues[i] = iterationData.errorValues[i + 1];
    }
    iterationData.errorValues[iterationData.errorValuesPoints - 1] = iterationData.minErrors;

    // Free threads
    delete[] threads;

    // Release the thread lock
    iterationData.threadLock = false;
}

void BeginSearching(const Timetable& timetable)
{
    // Print debug info
    LogInfo("Starting to search for the perfect timetable");
    std::cout << "Initializing timetables...\n";

    // Open the Generate timetable window
    iterationData = IterationData();
    iterationData.isDone = false;
    generateTimetableStatus = labels["Allocating memory for the timetables..."];
    isGenerateTimetable = true;
    wasGenerateTimetable = true;
    iterationData.iteration = -1;
    iterationData.threadLock = true;

    // Make a copy of settings
    iterationData.daysPerWeek = daysPerWeek;
    iterationData.lessonsPerDay = lessonsPerDay;
    iterationData.minTimetablesPerGeneration = minTimetablesPerGeneration;
    iterationData.maxTimetablesPerGeneration = maxTimetablesPerGeneration;

    // Initialize a starting population
    iterationData.timetables = new Timetable[iterationData.maxTimetablesPerGeneration];
    iterationData.population = new Timetable[iterationData.maxTimetablesPerGeneration];
    iterationData.newPopulation = new Timetable[iterationData.maxTimetablesPerGeneration];
    iterationData.timetablesPerGeneration = iterationData.maxTimetablesPerGeneration;
    for (int i = 0; i < iterationData.maxTimetablesPerGeneration; i++)
    {
        iterationData.timetables[i] = timetable;
        RandomizeTimetable(iterationData.timetables[i]);
        ScoreTimetable(iterationData.timetables[i]);
        iterationData.population[i] = iterationData.timetables[i];
        iterationData.newPopulation[i] = iterationData.timetables[i];
    }

    // Initialize the iteration variables
    iterationData.bestTimetableIndex = GetBestTimetableIndex(iterationData.timetables);
    iterationData.bestScore =
        EvaluateFitness(iterationData.timetables[iterationData.bestTimetableIndex]);
    iterationData.maxErrors = iterationData.timetables[iterationData.bestTimetableIndex].errors;
    iterationData.allTimeBestScore = iterationData.bestScore;
    iterationData.iteration = 0;
    for (size_t i = 0; i < iterationData.errorValuesPoints; i++)
    {
        iterationData.errorValues[i] = 0;
    }

    // Pre-cache class rule variants
    for (auto& classPair: timetable.classes)
    {
        for (size_t i = 0; i < classPair.second.timetableLessonRules.size(); i++)
        {
            iterationData.classRuleVariants[classPair.first].push_back(
                GetAllRuleVariants(classPair.second.timetableLessonRules[i]));
        }
    }

    // Release the thread lock
    iterationData.threadLock = false;

    // Run the iterations
    generateTimetableStatus = labels["Generating a timetable that matches the requirements..."];
    while (!iterationData.isDone)
        RunASearchIteration();
}

void StopSearching()
{
    LogInfo("Finished searching. The final timetable has " +
            std::to_string(iterationData.timetables[iterationData.bestTimetableIndex].errors) +
            " errors and " +
            std::to_string(iterationData.timetables[iterationData.bestTimetableIndex].bonusPoints) +
            " bonus points");
    iterationData.timetables[0].Save("timetables/" + iterationData.timetables[0].name + ".json");
    delete[] iterationData.timetables;
    delete[] iterationData.population;
    delete[] iterationData.newPopulation;
    iterationData.timetables = nullptr;
    iterationData.population = nullptr;
    iterationData.newPopulation = nullptr;
}

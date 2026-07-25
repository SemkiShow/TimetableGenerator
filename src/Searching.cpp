// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Searching.hpp"
#include "Logging.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "UI/Timetable/Generate.hpp"
#include <algorithm>
#include <chrono>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <map>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <utility>
#include <vector>

static std::random_device g_dev;
static thread_local std::mt19937 g_rng(g_dev());
size_t g_threadsNumber = std::max(1U, std::thread::hardware_concurrency());
IterationData g_iterationData;
static std::mutex g_iterationMutex;
constexpr int LOGGING_PERIOD = 10;

static int GetLessonsCount(const std::map<int, TimetableLesson>& timetableLessons)
{
    int output = 0;
    for (const auto& lesson: timetableLessons) output += lesson.second.count;
    return output;
}

static int GetLessonPlacesCount(const std::vector<Day>& days)
{
    int output = 0;
    for (int i = 0; i < g_iterationData.daysPerWeek; i++)
    {
        for (bool lesson: days[i].lessons)
        {
            if (lesson) output++;
        }
    }
    return output;
}

static bool IsTimetableValid(const Timetable& timetable)
{
    return std::all_of(timetable.classes.begin(), timetable.classes.end(),
                       [](const std::pair<int, Class>& classPair)
                       {
                           return GetLessonsCount(classPair.second.timetableLessons) <=
                                  GetLessonPlacesCount(classPair.second.days);
                       });
}

// Shuffle assign timetable lessons to classes
static void RandomizeTimetable(Timetable& timetable)
{
    if (!IsTimetableValid(timetable))
    {
        LOG_ERROR("The timetable is incorrect!");
        return;
    }
    for (auto& classPair: timetable.classes)
    {
        std::vector<int> timetableLessonIds;
        for (auto& lesson: classPair.second.timetableLessons)
        {
            for (int i = 0; i < lesson.second.count; i++)
                timetableLessonIds.push_back(lesson.first);
        }
        std::shuffle(timetableLessonIds.begin(), timetableLessonIds.end(), g_rng);
        size_t counter = 0;
        classPair.second.days.resize(g_iterationData.daysPerWeek);
        for (int i = 0; i < g_iterationData.daysPerWeek; i++)
        {
            classPair.second.days[i].classroomLessonPairs.clear();
            for (size_t j = 0; j < classPair.second.days[i].lessons.size(); j++)
            {
                classPair.second.days[i].classroomLessonPairs.push_back(ClassroomLessonPair());
                if (classPair.second.days[i].lessons[j])
                {
                    if (counter < timetableLessonIds.size())
                    {
                        classPair.second.days[i].classroomLessonPairs[j].timetableLessonId =
                            timetableLessonIds[counter];
                        for (size_t k = 0;
                             k < classPair.second.timetableLessons[timetableLessonIds[counter]]
                                     .lessonTeacherPairs.size();
                             k++)
                        {
                            int lessonId =
                                classPair.second.timetableLessons[timetableLessonIds[counter]]
                                    .lessonTeacherPairs[k]
                                    .lessonId;
                            int classroomId =
                                timetable.lessons[lessonId]
                                    .classroomIds[rand() %
                                                  timetable.lessons[lessonId].classroomIds.size()];
                            classPair.second.days[i].classroomLessonPairs[j].classroomIds.push_back(
                                classroomId);
                        }
                        counter++;
                    }
                    else
                        classPair.second.days[i].classroomLessonPairs[j].timetableLessonId =
                            ANY_LESSON;
                }
                else
                    classPair.second.days[i].classroomLessonPairs[j].timetableLessonId = NO_LESSON;
            }
        }
    }
}

static void SwapRandomTimetableLessons(Timetable& timetable)
{
    std::uniform_int_distribution<int> classDistribution(0,
                                                         (int)timetable.orderedClasses.size() - 1);
    std::uniform_int_distribution<int> dayDistribution(0, g_iterationData.daysPerWeek - 1);
    std::uniform_int_distribution<int> lesson1Distribution;
    std::uniform_int_distribution<int> lesson2Distribution;
    int classId;
    int classIndex;
    int lesson1Day;
    int lesson2Day;
    int lesson1Index;
    int lesson2Index;
    while (true)
    {
        // Get class id
        if (timetable.orderedClasses.empty()) LOG_ERROR("The timetable's classes are empty!");
        classIndex = classDistribution(g_rng);
        classId = timetable.orderedClasses[classIndex];

        // Get day id
        lesson1Day = dayDistribution(g_rng);
        lesson2Day = dayDistribution(g_rng);

        // Get ClassroomLessonPair id
        if (timetable.classes[classId].days[lesson1Day].classroomLessonPairs.empty() ||
            timetable.classes[classId].days[lesson2Day].classroomLessonPairs.empty())
            continue;
        lesson1Distribution = std::uniform_int_distribution<int>(
            0, (int)timetable.classes[classId].days[lesson1Day].classroomLessonPairs.size() - 1);
        lesson2Distribution = std::uniform_int_distribution<int>(
            0, (int)timetable.classes[classId].days[lesson2Day].classroomLessonPairs.size() - 1);
        lesson1Index = lesson1Distribution(g_rng);
        lesson2Index = lesson2Distribution(g_rng);

        // Exit if found a valid lesson
        if (timetable.classes[classId]
                    .days[lesson1Day]
                    .classroomLessonPairs[lesson1Index]
                    .timetableLessonId >= ANY_LESSON &&
            timetable.classes[classId]
                    .days[lesson2Day]
                    .classroomLessonPairs[lesson2Index]
                    .timetableLessonId >= ANY_LESSON)
            break;
    }
    // Swap 2 timetable lessons in the same class
    ClassroomLessonPair buf =
        timetable.classes[classId].days[lesson2Day].classroomLessonPairs[lesson2Index];
    timetable.classes[classId].days[lesson2Day].classroomLessonPairs[lesson2Index] =
        timetable.classes[classId].days[lesson1Day].classroomLessonPairs[lesson1Index];
    timetable.classes[classId].days[lesson1Day].classroomLessonPairs[lesson1Index] = buf;
}

static int EvaluateFitness(const Timetable& timetable)
{
    return timetable.bonusPoints - (int)((float)timetable.errors * g_settings.errorBonusRatio);
}

static Timetable TournamentSelection(const std::vector<Timetable>& population)
{
    const size_t TOURNAMENT_SIZE = 7;
    auto populationDistribution =
        std::uniform_int_distribution<int>(0, g_iterationData.timetablesPerGeneration - 1);
    int bestId = populationDistribution(g_rng);

    for (size_t i = 0; i < TOURNAMENT_SIZE; i++)
    {
        int challengerId = populationDistribution(g_rng);
        if (EvaluateFitness(population[challengerId]) > EvaluateFitness(population[bestId]))
            bestId = challengerId;
    }

    return population[bestId];
}

static Timetable Crossover(const Timetable& parent1, const Timetable& parent2)
{
    Timetable child = parent1;
    auto distribution2 = std::uniform_int_distribution<int>(0, 1);

    for (const auto& parent2Classes: parent2.classes)
    {
        if (distribution2(g_rng) == 0) child.classes[parent2Classes.first] = parent2Classes.second;
    }

    return child;
}

static void MutateTimetableClassroom(Timetable& timetable)
{
    std::uniform_int_distribution<int> classDistribution(0,
                                                         (int)timetable.orderedClasses.size() - 1);
    std::uniform_int_distribution<int> dayDistribution(0, g_iterationData.daysPerWeek - 1);
    std::uniform_int_distribution<int> lessonDistribution;
    std::uniform_int_distribution<int> classroomDistribution;
    std::uniform_int_distribution<int> lessonTeacherPairDistribution;
    while (true)
    {
        // Get class id
        if (timetable.orderedClasses.empty()) LOG_ERROR("The timetable's classes are empty!");
        int classIndex = classDistribution(g_rng);
        int classId = timetable.orderedClasses[classIndex];

        // Get day id
        int dayId = dayDistribution(g_rng);

        // Get ClassroomLessonPair id
        if (timetable.classes[classId].days[dayId].classroomLessonPairs.empty()) continue;
        lessonDistribution = std::uniform_int_distribution<int>(
            0, (int)timetable.classes[classId].days[dayId].classroomLessonPairs.size() - 1);
        int classroomLessonPairId = lessonDistribution(g_rng);

        // Get classroom id
        if (timetable.classes[classId]
                .days[dayId]
                .classroomLessonPairs[classroomLessonPairId]
                .classroomIds.empty())
            continue;
        classroomDistribution = std::uniform_int_distribution<int>(
            0, (int)timetable.classes[classId]
                       .days[dayId]
                       .classroomLessonPairs[classroomLessonPairId]
                       .classroomIds.size() -
                   1);
        int classroomId = classroomDistribution(g_rng);

        // Get timetable lesson id
        if (timetable.classes[classId]
                .days[dayId]
                .classroomLessonPairs[classroomLessonPairId]
                .timetableLessonId < 0)
            continue;
        int timetableLessonId = timetable.classes[classId]
                                    .days[dayId]
                                    .classroomLessonPairs[classroomLessonPairId]
                                    .timetableLessonId;
        if (timetableLessonId < 0) continue;

        // Get LessonTeacherPair id
        if (timetable.classes[classId]
                .timetableLessons[timetableLessonId]
                .lessonTeacherPairs.empty())
            continue;
        lessonTeacherPairDistribution =
            std::uniform_int_distribution<int>(0, (int)timetable.classes[classId]
                                                          .timetableLessons[timetableLessonId]
                                                          .lessonTeacherPairs.size() -
                                                      1);
        int lessonTeacherPairId = lessonTeacherPairDistribution(g_rng);

        // Generate a lessonId
        int lessonId = timetable.classes[classId]
                           .timetableLessons[timetableLessonId]
                           .lessonTeacherPairs[lessonTeacherPairId]
                           .lessonId;
        if (timetable.lessons[lessonId].classroomIds.empty()) continue;

        // Generate and replace a new classroom id
        classroomDistribution = std::uniform_int_distribution<int>(
            0, (int)timetable.lessons[lessonId].classroomIds.size() - 1);
        int newClassroomId = timetable.lessons[lessonId].classroomIds[classroomDistribution(g_rng)];
        timetable.classes[classId]
            .days[dayId]
            .classroomLessonPairs[classroomLessonPairId]
            .classroomIds[classroomId] = newClassroomId;
        return;
    }
}

static void MutateTimetable(Timetable& timetable)
{
    constexpr int MAX_SWAPS = 30;
    size_t swapsNumber = std::uniform_int_distribution<int>(0, MAX_SWAPS)(g_rng);
    for (size_t i = 0; i < swapsNumber; i++)
    {
        SwapRandomTimetableLessons(timetable);
    }

    constexpr int MAX_MUTATIONS = 10;
    size_t classroomsMutationsNumber = std::uniform_int_distribution<int>(0, MAX_MUTATIONS)(g_rng);
    for (size_t i = 0; i < classroomsMutationsNumber; i++)
    {
        MutateTimetableClassroom(timetable);
    }
}

static void GeneticAlgorithm(int threadId, std::vector<Timetable>& population,
                             std::vector<Timetable>& newPopulation)
{
    int firstId = threadId * g_iterationData.timetablesPerGeneration / (int)g_threadsNumber;
    int lastId = (threadId + 1) * g_iterationData.timetablesPerGeneration / (int)g_threadsNumber;
    for (int i = firstId; i < lastId; i++)
    {
        Timetable parent1 = TournamentSelection(population);
        Timetable parent2 = TournamentSelection(population);

        Timetable child = Crossover(parent1, parent2);
        MutateTimetable(child);
        if (g_settings.verboseLogging)
        {
            LogInfo("\x1b[32mScoring timetable %d\x1b[0m", i);
        }
        ScoreTimetable(child);
        newPopulation[i] = child;
    }
}

static void GetBestSpecies(std::vector<Timetable>& timetables, std::vector<Timetable>& population,
                           std::vector<Timetable>& newPopulation, int& minErrors)
{
    double averageFitness = 0;
    for (int i = 0; i < g_iterationData.timetablesPerGeneration; i++)
    {
        averageFitness += EvaluateFitness(population[i]);
        averageFitness += EvaluateFitness(newPopulation[i]);
    }
    averageFitness /= g_iterationData.timetablesPerGeneration * 2;

    int counter = 1;

    // Selecting the new population with above average fitness and minimal errors
    for (int i = 0; i < g_iterationData.timetablesPerGeneration; i++)
    {
        if (counter >= g_iterationData.timetablesPerGeneration) break;
        minErrors = std::min(newPopulation[i].errors, minErrors);
        if (EvaluateFitness(newPopulation[i]) >= averageFitness &&
            newPopulation[i].errors <= minErrors)
            timetables[counter++] = newPopulation[i];
    }

    // Selecting the new population with above average fitness and more than minimal errors
    for (int i = 0; i < g_iterationData.timetablesPerGeneration; i++)
    {
        if (counter >= g_iterationData.timetablesPerGeneration) break;
        minErrors = std::min(newPopulation[i].errors, minErrors);
        if (EvaluateFitness(newPopulation[i]) >= averageFitness &&
            newPopulation[i].errors > minErrors)
            timetables[counter++] = newPopulation[i];
    }

    // Selecting the old population with above average fitness and minimal errors
    for (int i = 0; i < g_iterationData.timetablesPerGeneration; i++)
    {
        if (counter >= g_iterationData.timetablesPerGeneration) break;
        minErrors = std::min(population[i].errors, minErrors);
        if (EvaluateFitness(population[i]) >= averageFitness && population[i].errors <= minErrors)
            timetables[counter++] = population[i];
    }

    // Selecting the old population with above average fitness and more than minimal errors
    for (int i = 0; i < g_iterationData.timetablesPerGeneration; i++)
    {
        if (counter >= g_iterationData.timetablesPerGeneration) break;
        minErrors = std::min(population[i].errors, minErrors);
        if (EvaluateFitness(population[i]) >= averageFitness && population[i].errors > minErrors)
            timetables[counter++] = population[i];
    }

    // Choosing the rest of the population randomly from the old and new populations
    for (int i = counter; i < g_iterationData.timetablesPerGeneration; i++)
    {
        if (std::uniform_int_distribution<int>(0, 1)(g_rng) == 0)
        {
            timetables[i] = population[std::uniform_int_distribution<int>(
                0, g_iterationData.timetablesPerGeneration - 1)(g_rng)];
        }
        else
        {
            timetables[i] = newPopulation[std::uniform_int_distribution<int>(
                0, g_iterationData.timetablesPerGeneration - 1)(g_rng)];
        }
    }

    if (g_iterationData.iteration % LOGGING_PERIOD == 0)
    {
        LogInfo("Selected %d/%d random timetables",
                g_iterationData.timetablesPerGeneration - counter,
                g_iterationData.timetablesPerGeneration);
    }
}

static int GetBestTimetableIndex(const std::vector<Timetable>& timetables)
{
    double bestTimetableScore = INT_MIN;
    int bestTimetableIndex = 0;
    double timetableScore = INT_MIN;
    for (int i = 0; i < g_iterationData.timetablesPerGeneration; i++)
    {
        timetableScore = EvaluateFitness(timetables[i]);
        g_iterationData.minErrors = std::min(timetables[i].errors, g_iterationData.minErrors);
        if (timetables[i].errors == 0 && timetables[i].bonusPoints > g_iterationData.maxBonusPoints)
        {
            g_iterationData.maxBonusPoints = timetables[i].bonusPoints;
        }
        if (timetableScore > bestTimetableScore &&
            timetables[i].errors <= g_iterationData.minErrors)
        {
            bestTimetableScore = timetableScore;
            bestTimetableIndex = i;
        }
    }
    return bestTimetableIndex;
}

static void InjectRandomImmigrants(std::vector<Timetable>& population)
{
    constexpr int MAX_IMMIGRANTS_PERCENT = 10;
    constexpr int ONE_HUNDRED = 100; // Just to tell clang-tidy to shut up
    int immigrantsCount = std::uniform_int_distribution<int>(
        0, g_iterationData.timetablesPerGeneration * MAX_IMMIGRANTS_PERCENT / ONE_HUNDRED)(g_rng);
    for (int i = 0; i < immigrantsCount; i++)
    {
        int idx = std::uniform_int_distribution<int>(1, g_iterationData.timetablesPerGeneration -
                                                            1)(g_rng);
        Timetable immigrant = g_currentTimetable;
        RandomizeTimetable(immigrant);
        ScoreTimetable(immigrant);
        population[idx] = immigrant;
    }
}

void RunASearchIteration()
{
    // Lock the mutex
    std::lock_guard<std::mutex> lock(g_iterationMutex);

    // Change the status is a timetable with 0 errors is found
    if (g_iterationData.timetables[g_iterationData.bestTimetableIndex].errors == 0)
    {
        if (g_iterationData.startBonusPoints == INT_MAX)
        {
            g_iterationData.startBonusPoints =
                g_iterationData.timetables[g_iterationData.bestTimetableIndex].bonusPoints;
        }
        g_generateTimetableMenu->SetStatus(GetText("Finding additional bonus points..."));
    }

    // Exit if there are the additional bonus points counter is over the limit or the iteratiuon
    // count is over the limit
    if (g_iterationData.timetables[g_iterationData.bestTimetableIndex].bonusPoints -
                g_iterationData.startBonusPoints >=
            g_settings.additionalBonusPoints ||
        (g_settings.maxIterations != -1 && g_iterationData.iteration >= g_settings.maxIterations))
    {
        g_iterationData.isDone = true;
        g_generateTimetableMenu->SetStatus(GetText("Timetable generating done!"));
        return;
    }

    // Init the threads
    std::vector<std::thread> threads(g_threadsNumber);

    g_iterationData.iteration++;

    // Output debug info
    if (g_iterationData.iteration % LOGGING_PERIOD == 0)
    {
        LogInfo("\x1b[34mIteration: %d\x1b[0m", g_iterationData.iteration);
        LogInfo("The best score is %d", g_iterationData.allTimeBestScore);
        LogInfo("The best timetable has %d errors",
                g_iterationData.timetables[g_iterationData.bestTimetableIndex].errors);
        LogInfo("The best timetable has %d bonus points",
                g_iterationData.timetables[g_iterationData.bestTimetableIndex].bonusPoints);
        LogInfo("%d iterations have passed since last score improvement",
                g_iterationData.iterationsPerChange);
    }

    // Change timetables per generation dynamically
    g_iterationData.timetablesPerGeneration =
        std::min(g_iterationData.maxTimetablesPerGeneration,
                 std::max(g_iterationData.minTimetablesPerGeneration,
                          (g_iterationData.iterationsPerChange + 1) *
                              g_settings.timetablesPerGenerationStep));

    // Inject random immigrants
    constexpr int IMMIGRANTS_PERIOD = 10;
    if (g_iterationData.iteration % IMMIGRANTS_PERIOD == 0)
    {
        InjectRandomImmigrants(g_iterationData.timetables);
    }

    // Run worker threads
    for (size_t i = 0; i < g_threadsNumber; i++)
    {
        threads[i] = std::thread(GeneticAlgorithm, i, std::ref(g_iterationData.timetables),
                                 std::ref(g_iterationData.newPopulation));
    }
    for (size_t i = 0; i < g_threadsNumber; i++) threads[i].join();

    // Get the best timetable from the current generation
    g_iterationData.bestTimetableIndex = GetBestTimetableIndex(g_iterationData.timetables);
    g_iterationData.bestScore =
        EvaluateFitness(g_iterationData.timetables[g_iterationData.bestTimetableIndex]);

    // Save the best timetable at index 0 (elitism)
    Timetable bufTimetable = g_iterationData.timetables[0];
    g_iterationData.timetables[0] = g_iterationData.timetables[g_iterationData.bestTimetableIndex];
    g_iterationData.timetables[g_iterationData.bestTimetableIndex] = bufTimetable;

    // Save only the best species from the old and new populations
    for (int i = 0; i < g_iterationData.timetablesPerGeneration; i++)
        g_iterationData.population[i] = g_iterationData.timetables[i];
    GetBestSpecies(g_iterationData.timetables, g_iterationData.population,
                   g_iterationData.newPopulation, g_iterationData.minErrors);

    // Change allTimeBestScore if current best score is better
    g_iterationData.allTimeBestScore =
        std::max(g_iterationData.bestScore, g_iterationData.allTimeBestScore);
    if (g_iterationData.lastAllTimeBestScore == g_iterationData.allTimeBestScore)
        g_iterationData.iterationsPerChange++;
    else
        g_iterationData.iterationsPerChange = 0;
    g_iterationData.lastAllTimeBestScore = g_iterationData.allTimeBestScore;
    g_iterationData.lastBestScore = g_iterationData.bestScore;

    // Update the error plot
    for (size_t i = 0; i < IterationData::ERROR_VALUES_SIZE - 1; i++)
    {
        g_iterationData.errorValues[i] = g_iterationData.errorValues[i + 1];
    }
    g_iterationData.errorValues[IterationData::ERROR_VALUES_SIZE - 1] =
        static_cast<float>(g_iterationData.minErrors);
}

void BeginSearching(const Timetable& timetable)
{
    // Lock the mutex
    // This has to be done manually, because RunASearchIteration() uses the same mutex, which causes
    // a deadlock, because the mutex isn't unlocked before running the function
    g_iterationMutex.lock();

    // Print debug info
    LogInfo("Starting to search for the perfect timetable");

    // Stop the previous search proces, if present
    if (!g_iterationData.isDone) StopSearching();

    // Open the Generate timetable window
    g_iterationData = {};
    g_iterationData.isDone = false;
    g_generateTimetableMenu->SetStatus(GetText("Allocating memory for the timetables..."));
    g_generateTimetableMenu->Open();
    // wasGenerateTimetable = true;

    // Make a copy of settings
    g_iterationData.daysPerWeek = g_settings.daysPerWeek;
    g_iterationData.lessonsPerDay = g_settings.lessonsPerDay;
    g_iterationData.minTimetablesPerGeneration = g_settings.minTimetablesPerGeneration;
    g_iterationData.maxTimetablesPerGeneration = g_settings.maxTimetablesPerGeneration;

    // Initialize a starting population
    g_iterationData.timetables.resize(g_iterationData.maxTimetablesPerGeneration);
    g_iterationData.population.resize(g_iterationData.maxTimetablesPerGeneration);
    g_iterationData.newPopulation.resize(g_iterationData.maxTimetablesPerGeneration);
    g_iterationData.timetablesPerGeneration = g_iterationData.maxTimetablesPerGeneration;
    for (int i = 0; i < g_iterationData.maxTimetablesPerGeneration; i++)
    {
        g_iterationData.timetables[i] = timetable;
        RandomizeTimetable(g_iterationData.timetables[i]);
        ScoreTimetable(g_iterationData.timetables[i]);
        g_iterationData.population[i] = g_iterationData.timetables[i];
        g_iterationData.newPopulation[i] = g_iterationData.timetables[i];
    }

    // Initialize the iteration variables
    g_iterationData.bestTimetableIndex = GetBestTimetableIndex(g_iterationData.timetables);
    g_iterationData.bestScore =
        EvaluateFitness(g_iterationData.timetables[g_iterationData.bestTimetableIndex]);
    g_iterationData.maxErrors =
        g_iterationData.timetables[g_iterationData.bestTimetableIndex].errors;
    g_iterationData.allTimeBestScore = g_iterationData.bestScore;
    g_iterationData.iteration = 0;
    for (size_t i = 0; i < IterationData::ERROR_VALUES_SIZE; i++)
    {
        g_iterationData.errorValues[i] = 0;
    }

    // Empty classes failsafe
    if (timetable.classes.size() == 0)
    {
        LogInfo("The timetable has no classes!");
        g_iterationData.isDone = true;
        g_generateTimetableMenu->SetStatus(GetText("Timetable generating done!"));
        return;
    }

    // Pre-cache class rule variants
    for (const auto& classPair: timetable.classes)
    {
        for (size_t i = 0; i < classPair.second.timetableLessonRules.size(); i++)
        {
            g_iterationData.classRuleVariants[classPair.first].push_back(
                GetAllRuleVariants(classPair.second.timetableLessonRules[i]));
        }
    }

    // Unlock the mutex
    g_iterationMutex.unlock();

    // Run the iterations
    g_generateTimetableMenu->SetStatus(
        GetText("Generating a timetable that matches the requirements..."));
    while (!g_iterationData.isDone)
    {
        RunASearchIteration();
        // Avoid always locking iterationMutex
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void StopSearching()
{
    // Lock the mutex
    std::lock_guard<std::mutex> lock(g_iterationMutex);

    LogInfo("Finished searching. The final timetable has %d errors and %d bonus points",
            g_iterationData.timetables[g_iterationData.bestTimetableIndex].errors,
            g_iterationData.timetables[g_iterationData.bestTimetableIndex].bonusPoints);
    g_iterationData.isDone = true;
    g_iterationData.timetables[0].Save("timetables/" + g_iterationData.timetables[0].name +
                                       ".json");
    g_iterationData.timetables.clear();
    g_iterationData.population.clear();
    g_iterationData.newPopulation.clear();
}

void ToggleVerboseLoggingThreads()
{
    // Lock the mutex
    std::lock_guard<std::mutex> lock(g_iterationMutex);

    if (g_settings.verboseLogging)
        g_threadsNumber = 1;
    else
        g_threadsNumber = std::max(1U, std::thread::hardware_concurrency());
}

#include "Searching.hpp"
#include "Settings.hpp"
#include "UI.hpp"

static thread_local std::mt19937 rng(std::random_device{}());
int threadsNumber = std::max(std::thread::hardware_concurrency(), (unsigned int)1);
IterationData iterationData = IterationData();

int GetLessonsAmount(const std::map<int, TimetableLesson> timetableLessons)
{
    int output = 0;
    for (auto& lesson: timetableLessons)
        output += lesson.second.amount;
    return output;
}

int GetLessonPlacesAmount(const Day days[DAYS_PER_WEEK])
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

bool IsTimetableCorrect(const Timetable* timetable)
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
        for (auto& lesson: classPair.second.timetableLessons)
        {
            for (int i = 0; i < lesson.second.amount; i++)
                timetableLessonIDs.push_back(lesson.first);
        }
        std::shuffle(timetableLessonIDs.begin(), timetableLessonIDs.end(), rng);
        int counter = 0;
        for (int i = 0; i < DAYS_PER_WEEK; i++)
        {
            classPair.second.days[i].classroomLessonPairs.clear();
            for (int j = 0; j < classPair.second.days[i].lessons.size(); j++)
            {
                classPair.second.days[i].classroomLessonPairs.push_back(ClassroomLessonPair());
                if (classPair.second.days[i].lessons[j])
                {
                    if (counter < timetableLessonIDs.size())
                    {
                        classPair.second.days[i].classroomLessonPairs[j].timetableLessonID = timetableLessonIDs[counter];
                        for (int k = 0; k < classPair.second.timetableLessons[timetableLessonIDs[counter]].lessonTeacherPairs.size(); k++)
                        {
                            int lessonID = classPair.second.timetableLessons[timetableLessonIDs[counter]].lessonTeacherPairs[k].lessonID;
                            int classroomID = timetable->lessons[lessonID].classroomIDs[rand() % timetable->lessons[lessonID].classroomIDs.size()];
                            classPair.second.days[i].classroomLessonPairs[j].classroomIDs.push_back(classroomID);
                        }
                        counter++;
                    }
                    else classPair.second.days[i].classroomLessonPairs[j].timetableLessonID = -2;
                }
                else classPair.second.days[i].classroomLessonPairs[j].timetableLessonID = -3;
            }
        }
    }
}

std::uniform_int_distribution<int> dayDistribution(0, DAYS_PER_WEEK - 1);
void SwapRandomTimetableLessons(Timetable* timetable)
{
    std::uniform_int_distribution<int> classDistribution(0, timetable->orderedClasses.size() - 1);
    std::uniform_int_distribution<int> lesson1Distribution;
    std::uniform_int_distribution<int> lesson2Distribution;
    int classID, classIndex, lesson1Day, lesson2Day, lesson1Index, lesson2Index;
    while (true)
    {
        if (timetable->orderedClasses.empty()) std::cerr << "The timetable's classes are empty!\n";
        classIndex = classDistribution(rng);
        classID = timetable->orderedClasses[classIndex];

        lesson1Day = dayDistribution(rng);
        lesson2Day = dayDistribution(rng);

        if (timetable->classes[classID].days[lesson1Day].classroomLessonPairs.empty() ||
        timetable->classes[classID].days[lesson2Day].classroomLessonPairs.empty()) continue;

        lesson1Distribution = std::uniform_int_distribution<int>(0, timetable->classes[classID].days[lesson1Day].classroomLessonPairs.size() - 1);
        lesson2Distribution = std::uniform_int_distribution<int>(0, timetable->classes[classID].days[lesson2Day].classroomLessonPairs.size() - 1);
        lesson1Index = lesson1Distribution(rng);
        lesson2Index = lesson2Distribution(rng);

        if (timetable->classes[classID].days[lesson1Day].classroomLessonPairs[lesson1Index].timetableLessonID >= -2 &&
        timetable->classes[classID].days[lesson2Day].classroomLessonPairs[lesson2Index].timetableLessonID >= -2) break;
    }
    ClassroomLessonPair buf = timetable->classes[classID].days[lesson2Day].classroomLessonPairs[lesson2Index];
    timetable->classes[classID].days[lesson2Day].classroomLessonPairs[lesson2Index] = timetable->classes[classID].days[lesson1Day].classroomLessonPairs[lesson1Index];
    timetable->classes[classID].days[lesson1Day].classroomLessonPairs[lesson1Index] = buf;
}

void SimulatedAnnealing(int threadID, Timetable* timetables, double temperature)
{
    for (int i = threadID * timetablesPerGeneration / threadsNumber; i < (threadID + 1) * timetablesPerGeneration / threadsNumber; i++)
    {
        // Mutate timetable
        if (i != 0)
        {
            timetables[i] = timetables[0];
            int changesAmont = std::max(1, (int)(maxMutations * temperature / maxTemperature));
            for (int j = 0; j < changesAmont; j++)
            {
                SwapRandomTimetableLessons(&timetables[i]);
            }
        }

        // Score timetable
        ScoreTimetable(&timetables[i]);
        int timetableScore = timetables[i].bonusPoints - timetables[i].errors;
        // std::cout << "Calculated timetable " << i << '\n';
    }
}

double EvaluateFitness(const Timetable& timetable)
{
    // return timetable.bonusPoints / (1 + timetable.errors * 3.0);
    return timetable.bonusPoints - (timetable.errors * errorBonusRatio);
}

Timetable TournamentSelection(const Timetable* population)
{
    int tournamentSize = 7;
    auto populationDistribution = std::uniform_int_distribution<int>(0, timetablesPerGeneration-1);
    int bestID = populationDistribution(rng);

    for (int i = 0; i < tournamentSize; i++)
    {
        int challengerID = populationDistribution(rng);
        if (EvaluateFitness(population[challengerID]) > EvaluateFitness(population[bestID])) bestID = challengerID;
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

void MutateTimetableClassroom(Timetable* timetable)
{
    std::uniform_int_distribution<int> classDistribution(0, timetable->orderedClasses.size() - 1);
    std::uniform_int_distribution<int> lessonDistribution;
    std::uniform_int_distribution<int> classroomDistribution;
    std::uniform_int_distribution<int> lessonTeacherPairDistribution;
    int classID, classIndex, dayID, classroomLessonPairID, lessonTeacherPairID, classroomID;
    while (true)
    {
        if (timetable->orderedClasses.empty()) std::cerr << "The timetable's classes are empty!\n";
        classIndex = classDistribution(rng);
        classID = timetable->orderedClasses[classIndex];

        dayID = dayDistribution(rng);

        if (timetable->classes[classID].days[dayID].classroomLessonPairs.empty()) continue;

        lessonDistribution = std::uniform_int_distribution<int>(
            0, timetable->classes[classID].days[dayID].classroomLessonPairs.size() - 1);
        classroomLessonPairID = lessonDistribution(rng);

        if (timetable->classes[classID].days[dayID].classroomLessonPairs[classroomLessonPairID].classroomIDs.empty()) continue;
        classroomDistribution = std::uniform_int_distribution<int>(
            0, timetable->classes[classID].days[dayID].classroomLessonPairs[classroomLessonPairID].classroomIDs.size() - 1);
        int classroomID = classroomDistribution(rng);

        if (timetable->classes[classID].days[dayID].classroomLessonPairs[classroomLessonPairID].timetableLessonID < 0) continue;
        int timetableLessonID = timetable->classes[classID].days[dayID].classroomLessonPairs[classroomLessonPairID].timetableLessonID;
        if (timetableLessonID < 0) continue;

        if (timetable->classes[classID].timetableLessons[timetableLessonID].lessonTeacherPairs.empty()) continue;
        lessonTeacherPairDistribution = std::uniform_int_distribution<int>(
            0, timetable->classes[classID].timetableLessons[timetableLessonID].lessonTeacherPairs.size() - 1);
        lessonTeacherPairID = lessonTeacherPairDistribution(rng);

        int lessonID = timetable->classes[classID].timetableLessons[timetableLessonID].lessonTeacherPairs[lessonTeacherPairID].lessonID;
        classroomDistribution = std::uniform_int_distribution<int>(
            0, timetable->lessons[lessonID].classroomIDs.size() - 1);
        int newClassroomID = timetable->lessons[lessonID].classroomIDs[classroomDistribution(rng)];
        timetable->classes[classID].days[dayID].classroomLessonPairs[classroomLessonPairID].classroomIDs[classroomID] = newClassroomID;
        return;
    }
}

void MutateTimetable(Timetable* timetable)
{
    int swapsAmount = std::uniform_int_distribution<int>(0, 30)(rng);
    for (int i = 0; i < swapsAmount; i++)
    {
        SwapRandomTimetableLessons(timetable);
    }
    int classroomsMutationsAmount = std::uniform_int_distribution<int>(0, 10)(rng);
    for (int i = 0; i < classroomsMutationsAmount; i++)
    {
        MutateTimetableClassroom(timetable);
    }
}

void GeneticAlgorithm(int threadID, Timetable* population, Timetable* newPopulation)
{
    for (int i = threadID * timetablesPerGeneration / threadsNumber; i < (threadID + 1) * timetablesPerGeneration / threadsNumber; i++)
    {
        Timetable parent1 = TournamentSelection(population);
        Timetable parent2 = TournamentSelection(population);

        Timetable child = Crossover(parent1, parent2);
        MutateTimetable(&child);
    #ifdef VERBOSE_LOGGING
        std::cout << "\x1b[32mScoring timetable " << i << "\x1b[0m. ";
    #endif
        ScoreTimetable(&child);
        newPopulation[i] = child;
    }
}

void GetBestSpecies(Timetable* timetables, Timetable* population, Timetable* newPopulation, int* minErrors)
{
    double averageFitness = 0;
    for (int i = 0; i < timetablesPerGeneration; i++)
    {
        averageFitness += EvaluateFitness(population[i]);
        averageFitness += EvaluateFitness(newPopulation[i]);
    }
    averageFitness /= timetablesPerGeneration * 2;

    int counter = 1;
    // Selecting the new population with above average fitness and minimal errors
    for (int i = 0; i < timetablesPerGeneration; i++)
    {
        if (counter >= timetablesPerGeneration) break;
        if (newPopulation[i].errors < *minErrors) *minErrors = newPopulation[i].errors;
        if (EvaluateFitness(newPopulation[i]) >= averageFitness && newPopulation[i].errors <= *minErrors)
            timetables[counter++] = newPopulation[i];
    }
    // Selecting the new population with above average fitness and more than minimal errors
    for (int i = 0; i < timetablesPerGeneration; i++)
    {
        if (counter >= timetablesPerGeneration) break;
        if (newPopulation[i].errors < *minErrors) *minErrors = newPopulation[i].errors;
        if (EvaluateFitness(newPopulation[i]) >= averageFitness && newPopulation[i].errors > *minErrors)
            timetables[counter++] = newPopulation[i];
    }
    // Selecting the old population with above average fitness and minimal errors
    for (int i = 0; i < timetablesPerGeneration; i++)
    {
        if (counter >= timetablesPerGeneration) break;
        if (population[i].errors < *minErrors) *minErrors = population[i].errors;
        if (EvaluateFitness(population[i]) >= averageFitness && population[i].errors <= *minErrors)
            timetables[counter++] = population[i];
    }
    // Selecting the old population with above average fitness and more than minimal errors
    for (int i = 0; i < timetablesPerGeneration; i++)
    {
        if (counter >= timetablesPerGeneration) break;
        if (population[i].errors < *minErrors) *minErrors = population[i].errors;
        if (EvaluateFitness(population[i]) >= averageFitness && population[i].errors > *minErrors)
            timetables[counter++] = population[i];
    }
    // Choosing the rest of the population randomly from the old and new populations
    for (int i = counter; i < timetablesPerGeneration; i++)
    {
        if (std::uniform_int_distribution<int>(0, 1)(rng) == 0)
        {
            timetables[i] = population[std::uniform_int_distribution<int>(0, timetablesPerGeneration-1)(rng)];
        }
        else
        {
            timetables[i] = newPopulation[std::uniform_int_distribution<int>(0, timetablesPerGeneration-1)(rng)];
        }
    }
    std::cout << "Selected " << timetablesPerGeneration - counter << "/" << timetablesPerGeneration << " random timetables. ";
}

int GetBestTimetableIndex(const Timetable* timetables, int* minErrors)
{
    double bestTimetableScore = INT_MIN;
    int bestTimetableIndex = 0;
    double timetableScore = INT_MIN;
    for (int i = 0; i < timetablesPerGeneration; i++)
    {
    #ifdef GENETIC_ALGORITHM
        timetableScore = EvaluateFitness(timetables[i]);
    #endif
    #ifdef SIMULATED_ANNEALING
        timetableScore = timetables[i].bonusPoints - timetables[i].errors;
    #endif
        if (timetables[i].errors < *minErrors) *minErrors = timetables[i].errors;
        if (timetableScore > bestTimetableScore && timetables[i].errors <= *minErrors)
        {
            bestTimetableScore = timetableScore;
            bestTimetableIndex = i;
        }
    }
    return bestTimetableIndex;
}

void BeginSearching(const Timetable* timetable)
{
    std::cout << "Initializing timetables...\n";
    iterationData = IterationData();
    iterationData.timetables = new Timetable[timetablesPerGeneration];
#ifdef GENETIC_ALGORITHM
    iterationData.population = new Timetable[timetablesPerGeneration];
    iterationData.newPopulation = new Timetable[timetablesPerGeneration];
#endif
    for (int i = 0; i < timetablesPerGeneration; i++)
    {
        iterationData.timetables[i] = *timetable;
        RandomizeTimetable(&iterationData.timetables[i]);
        ScoreTimetable(&iterationData.timetables[i]);
    #ifdef GENETIC_ALGORITHM
        iterationData.population[i] = iterationData.timetables[i];
        iterationData.newPopulation[i] = iterationData.timetables[i];
    #endif
    }

    iterationData.bestTimetableIndex = GetBestTimetableIndex(iterationData.timetables, &iterationData.minErrors);
    iterationData.bestScore = EvaluateFitness(iterationData.timetables[iterationData.bestTimetableIndex]);
    iterationData.allTimeBestScore = iterationData.bestScore;
    iterationData.temperature = maxTemperature;
    iterationData.isDone = false;
    isGenerateTimetable = true;
    threadsNumber = 1;
}

void RunASearchIteration()
{
    // if (iterationData.timetables[iterationData.bestTimetableIndex].errors <= 0)
    if (iterationData.iteration >= 100)
    {
        iterationData.isDone = true;
        SaveTimetable("timetables/" + iterationData.timetables[0].name + ".json", &iterationData.timetables[0]);
        return;
    }
    std::thread threads[threadsNumber];

    std::cout << "\x1b[34mIteration " << iterationData.iteration++ << "\x1b[0m. ";
    std::cout << "The current best score is " << iterationData.bestScore << ". ";
    std::cout << "The best score is " << iterationData.allTimeBestScore << ". ";
    std::cout << "The best timetable has " << iterationData.timetables[iterationData.bestTimetableIndex].errors << " errors. ";
    std::cout << "The best timetable index is " << iterationData.bestTimetableIndex << ". ";
#ifdef SIMULATED_ANNEALING
    std::cout << "The temperature is " << temperature << ". ";
#endif
    std::cout << iterationData.iterationsPerChange << " iterations have passed since last score improvement. ";
#ifdef GENETIC_ALGORITHM
    for (int i = 0; i < threadsNumber; i++)
        threads[i] = std::thread(GeneticAlgorithm, i, iterationData.timetables, iterationData.newPopulation);
    for (int i = 0; i < threadsNumber; i++)
        threads[i].join();
#else
#ifdef SIMULATED_ANNEALING
    for (int i = 0; i < threadsNumber; i++)
        threads[i] = std::thread(SimulatedAnnealing, i, iterationData.timetables, iterationData.temperature);
    for (int i = 0; i < threadsNumber; i++)
        threads[i].join();
#endif
#endif
    iterationData.bestTimetableIndex = GetBestTimetableIndex(iterationData.timetables, &iterationData.minErrors);
    iterationData.bestScore = EvaluateFitness(iterationData.timetables[iterationData.bestTimetableIndex]);
#ifdef SIMULATED_ANNEALING
    int delta = bestScore - lastBestScore;
    if (delta > 0 || std::exp((double)delta / temperature) > (double)rand() / RAND_MAX)
    {
        Timetable bufTimetable = iterationData.timetables[0];
        iterationData.timetables[0] = iterationData.timetables[bestTimetableIndex];
        iterationData.timetables[iterationData.bestTimetableIndex] = bufTimetable;
        std::cout << "Choosing the best timetable";
    }
    else
    {
        int randomTimetableIndex = rand() % timetablesPerGeneration;
        Timetable bufTimetable = timetables[0];
        timetables[0] = timetables[randomTimetableIndex];
        timetables[randomTimetableIndex] = bufTimetable;
        std::cout << "Choosing a random timetable";
    }
    temperature = maxTemperature * std::pow(coolingRate, iteration);
#endif
#ifdef GENETIC_ALGORITHM
    Timetable bufTimetable = iterationData.timetables[0];
    iterationData.timetables[0] = iterationData.timetables[iterationData.bestTimetableIndex];
    iterationData.timetables[iterationData.bestTimetableIndex] = bufTimetable;
    for (int i = 0; i < timetablesPerGeneration; i++)
        iterationData.population[i] = iterationData.timetables[i];
    GetBestSpecies(iterationData.timetables, iterationData.population, iterationData.newPopulation, &iterationData.minErrors);
#endif
    std::cout << '\n';
    if (iterationData.bestScore > iterationData.allTimeBestScore) iterationData.allTimeBestScore = iterationData.bestScore;
    if (iterationData.lastAllTimeBestScore == iterationData.allTimeBestScore) iterationData.iterationsPerChange++;
    else iterationData.iterationsPerChange = 0;
    iterationData.lastAllTimeBestScore = iterationData.allTimeBestScore;
    iterationData.lastBestScore = iterationData.bestScore;
}

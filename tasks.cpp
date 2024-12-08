#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <unordered_map>
#include <cmath>
#include <iomanip>

using namespace std;

class NumberGenerator {
protected:
    random_device rd;
    mt19937 gen;

public:
    NumberGenerator() : gen(rd()) {}
    virtual double generate() = 0;
    virtual ~NumberGenerator() = default;
};

class IntGenerator : public NumberGenerator {
protected:
    uniform_int_distribution<> dis;

public:
    IntGenerator(int min, int max) : dis(min, max) {}
    double generate() override { return dis(gen); }
};

class RealGenerator : public NumberGenerator {
protected:
    uniform_real_distribution<> dis;

public:
    RealGenerator(double min, double max) : dis(min, max) {}
    double generate() override { return dis(gen); }
};



// Фабрика для генерации координат
class CoordinateGenerator {
protected:

    shared_ptr<NumberGenerator> xGen;
    shared_ptr<NumberGenerator> yGen;

public:
    CoordinateGenerator(shared_ptr<NumberGenerator> xGen, shared_ptr<NumberGenerator> yGen)
        : xGen(xGen), yGen(yGen) {}

    pair<double, double> generate() {
        return { xGen->generate(), yGen->generate() };
    }
};



// Базовый класс для небесных тел
class CelestialBody {
protected:
    string name;
    double x, y;
    int productivity;
    double price; 

public:
    CelestialBody(const string& name, double x, double y, int productivity, double price)
        : name(name), x(x), y(y), productivity(productivity), price(price) {}

    virtual void info() const {
        cout << name << " at (" << x << ", " << y << "), production: " << productivity << ", price: " << price;
    }

    string getName() const { return name; }
    double getX() const { return x; }
    double getY() const { return y; }
    double getPrice() const { return price; }
    int getProductivity() const { return productivity; }

    virtual ~CelestialBody() = default;
};



class Planet;

class Asteroid : public CelestialBody {
protected:

    int asteroidProductivity;
    shared_ptr<CelestialBody> owner;

public:
    Asteroid(const string& name, double x, double y, int productivity, double price, int asteroidProductivity)
        : CelestialBody(name, x, y, productivity, price), asteroidProductivity(asteroidProductivity) {}

    double calculateFuelCost(shared_ptr<Planet> planet);

    void info() const override {
        CelestialBody::info();
        cout << " | Productivity: " << asteroidProductivity << "\n";
    }

    void setOwner(shared_ptr<CelestialBody> owner) { this->owner = owner; }
    void setPrice(double newPrice) { price = newPrice; }
    shared_ptr<CelestialBody> getOwner() const { return owner; }
};

class Planet : public CelestialBody, public enable_shared_from_this<Planet> {
protected:

    int demand;
    string technologyLevel;
    int accumulatedResources;
    double money;
    double fuel;
    double fuelCost;
    vector<shared_ptr<Asteroid>> asteroids;
    shared_ptr<IntGenerator> demandGen;

public:
    Planet(const string& name, double x, double y, int productivity, int demand, string technologyLevel, double money, double fuel, double fuelCost, double price)
        : CelestialBody(name, x, y, productivity, price), demand(demand), technologyLevel(technologyLevel), accumulatedResources(0), money(money), fuel(fuel), fuelCost(fuelCost), demandGen(make_shared<IntGenerator>(500, 1500)) {}

    void showOwnedAsteroids() const {
        if (!asteroids.empty()) {
            cout << name << " owns the following asteroids:\n";
            for (const auto& asteroid : asteroids) {
                cout << "  - " << asteroid->getName() << "\n";
            }
        } else {
            cout << name << " does not own any asteroids.\n";
        }
    }

    void info() const override {
        CelestialBody::info();
        cout << " | Accumulated: " << accumulatedResources << " | Demand: " << demand
            << " | Technology: " << technologyLevel << " | Money: " << money
            << " | Fuel: " << fuel << "\n";
        showOwnedAsteroids(); 
    }

    double getFuelCost() const { return fuelCost; }
    string getTechnologyLevel() const { return technologyLevel; }
    int getDemand() const { return demand; }
    double getMoney() const { return money; }
    void setFuelCost(double newFuelCost) { fuelCost = newFuelCost; }
    void setDemand(int newDemand) { demand = newDemand; }

    void generateResources() {
        int generatedResources = productivity;
        accumulatedResources += productivity;
        double moneyGenerated = generatedResources * 0.125;
        double fuelGenerated = generatedResources * 0.0625;
        money += moneyGenerated;
        fuel += fuelGenerated;

        cout << name << " generated " << productivity << " resources. Total: " << accumulatedResources << " / " << demand
            << " and converted resources to " << moneyGenerated << " money and " << fuelGenerated << " fuel.\n";

        if (accumulatedResources >= demand) {
            upgradeTechnology();
        }
    }

    void upgradeTechnology() {
        accumulatedResources = 0;
        money = 0;
        fuel = 0;

        if (technologyLevel == "low") {
            technologyLevel = "medium";
            demand = demandGen->generate();
            productivity *= 1.25;
        } else if (technologyLevel == "medium") {
            technologyLevel = "high";
            demandGen = make_shared<IntGenerator>(1700, 2100); 
            demand = demandGen->generate(); 
            productivity *= 1.5;
        } else {
            cout << name << " has reached the maximum technology level!\n";
            productivity *= 1.75;
            demand = 0;
        }

        cout << name << " upgraded to " << technologyLevel
            << " technology level. New demand: " << demand
            << ", Productivity: " << productivity << "\n";
    }

    void buyAsteroid(shared_ptr<Asteroid> asteroid) {
        if (auto self = shared_from_this()) { // Проверяем, что объект управляется shared_ptr
            double fuelCostToAsteroid = asteroid->calculateFuelCost(self);
            if (money >= asteroid->getPrice() && fuel >= fuelCostToAsteroid) {
                money -= asteroid->getPrice();
                fuel -= fuelCostToAsteroid;
                asteroids.push_back(asteroid);
                asteroid->setOwner(self); 
                productivity += asteroid->getProductivity();  
                cout << name << " successfully bought " << asteroid->getName() << "!\n";
                cout << name << " productivity increased to " << productivity << "\n";
                cout << "Fuel cost to reach " << asteroid->getName() << ": " << fuelCostToAsteroid << "\n";
                showOwnedAsteroids(); // Выводим информацию о владении астероидами

                // Обнуляем все значения
                accumulatedResources = 0;
                money = 0;
                fuel = 0;
            } else {
                cout << name << " cannot buy " << asteroid->getName() << " due to insufficient funds or fuel.\n";
                cout << "Fuel cost to reach " << asteroid->getName() << ": " << fuelCostToAsteroid << "\n";
            }
        } else {
            throw runtime_error("Planet is not managed by shared_ptr.");
        }
    }
};

double Asteroid::calculateFuelCost(shared_ptr<Planet> planet) {
    double xDiff = planet->getX() - getX();
    double yDiff = planet->getY() - getY();
    double distance = sqrt(xDiff * xDiff + yDiff * yDiff);
    return distance * planet->getFuelCost();
}



// Базовый класс для корпораций
class Corporation {
protected:
    string name;
    shared_ptr<IntGenerator> chanceGen;
    
public:
    Corporation(const string& name) : name(name), chanceGen(make_shared<IntGenerator>(0, 100)) {}    
    virtual void operate(vector<shared_ptr<Planet>>& planets, vector<shared_ptr<Asteroid>>& asteroids) = 0;
    virtual ~Corporation() = default;
};

class LogisticsCompany : public Corporation {
public:
    LogisticsCompany(const string& name) : Corporation(name) {}

    void operate(vector<shared_ptr<Planet>>& planets, vector<shared_ptr<Asteroid>>& asteroids) override {
        if (chanceGen->generate() < 20) { 
            // Логика минимизации транспортных расходов
            cout << name << " is optimizing transportation costs!!! 'Reducing the cost of fuel'\n";
            for (auto& planet : planets) {
                planet->setFuelCost(planet->getFuelCost() * 0.9); // Уменьшаем стоимость топлива на 10%
            }
        }
    }
};

class TechTrader : public Corporation {
public:
    TechTrader(const string& name) : Corporation(name) {}

    void operate(vector<shared_ptr<Planet>>& planets, vector<shared_ptr<Asteroid>>& asteroids) override {
        if (chanceGen->generate() < 20) { 
            // Логика торговли высокотехнологичными ресурсами
            cout << name << " is trading high-tech resources.\n";
            for (auto& planet : planets) {
                if (planet->getTechnologyLevel() == "high" || planet->getTechnologyLevel() == "medium") {
                    planet->setDemand(planet->getDemand() * 1.1); // Увеличиваем спрос на 10% для высокотехнологичных планет
                }
            }
        }
    }
};

class MinerCompany : public Corporation {
public:
    MinerCompany(const string& name) : Corporation(name) {}

    void operate(vector<shared_ptr<Planet>>& planets, vector<shared_ptr<Asteroid>>& asteroids) override {
        if (chanceGen->generate() < 20) { // 25% шанс
            // Логика добычи ресурсов на астероидах
            cout << name << " is mining resources from asteroids!!! 'Reducing the cost of asteroids' \n";
            for (auto& asteroid : asteroids) {
                if (!asteroid->getOwner()) {
                    asteroid->setPrice(asteroid->getPrice() * 0.9); // Уменьшаем стоимость астероидов на 10%
                }
            }
        }
    }
};



// Класс для игрового поля
class GameField {
protected:

    static const int MAX_PLANETS = 100;
    static const int MAX_ASTEROIDS = 50;

    int numPlanets;
    int numAsteroids;
    int numRoutes;

    vector<shared_ptr<Planet>> planets;
    vector<shared_ptr<Asteroid>> asteroids;
    unordered_map<string, unordered_map<string, double>> fuelCosts;
    shared_ptr<CoordinateGenerator> coordGen;
    vector<shared_ptr<Corporation>> corporations;

public:
    GameField(int numPlanets, int numAsteroids, int numRoutes, shared_ptr<CoordinateGenerator> coordGen)
        : numPlanets(numPlanets), numAsteroids(numAsteroids), numRoutes(numRoutes), coordGen(coordGen) {
        if (numPlanets > MAX_PLANETS || numAsteroids > MAX_ASTEROIDS) {
            throw invalid_argument("Too many planets or asteroids!");
        }
    }

    void generateField() {
        shared_ptr<IntGenerator> prodGen = make_shared<IntGenerator>(37, 60);
        shared_ptr<IntGenerator> demandGen = make_shared<IntGenerator>(250, 500);
        shared_ptr<RealGenerator> priceGen = make_shared<RealGenerator>(150.0, 300.0);

        for (int i = 0; i < numPlanets; i++) {
            auto [x, y] = coordGen->generate();
            int productivity = prodGen->generate();
            int demand = demandGen->generate();
            double price = priceGen->generate();
            planets.push_back(make_shared<Planet>("Planet-" + to_string(i + 1), x, y, productivity, demand, "low", 0.0, 0.0, 0.1, price));
        }

        for (int i = 0; i < numAsteroids; i++) {
            auto [x, y] = coordGen->generate();
            int productivity = prodGen->generate();
            double price = priceGen->generate();
            asteroids.push_back(make_shared<Asteroid>("Asteroid-" + to_string(i + 1), x, y, productivity, price, productivity));
        }

        for (int i = 0; i < numPlanets; i++) {
            for (int j = i + 1; j < numPlanets; j++) {
                double distance = calculateDistance(planets[i], planets[j]);
                double fuelCost = distance * 0.1;
                fuelCosts[planets[i]->getName()][planets[j]->getName()] = fuelCost;
                fuelCosts[planets[j]->getName()][planets[i]->getName()] = fuelCost;
            }
        }
    }

    double calculateDistance(shared_ptr<CelestialBody> from, shared_ptr<CelestialBody> to) {
        double xDiff = from->getX() - to->getX();
        double yDiff = from->getY() - to->getY();
        return sqrt(xDiff * xDiff + yDiff * yDiff);
    }

    void showInfo() const {
        cout << "Planets:\n";
        for (const auto& planet : planets) {
            planet->info();
        }
        cout << "Asteroids:\n";
        for (const auto& asteroid : asteroids) {
            asteroid->info();
        }
    }

    void runSimulation(int steps) {
        for (int i = 0; i < steps; i++) {
            cout << "Step " << i + 1 << ":\n";
            for (auto& planet : planets) {
                planet->generateResources();
                for (auto& asteroid : asteroids) {
                    if (!asteroid->getOwner()) {
                        planet->buyAsteroid(asteroid);
                    }
                }
            }
            for (auto& corporation : corporations) {
                corporation->operate(planets, asteroids);
            }
            cout << "-------------------\n";
            showInfo();
            cout << "-------------------\n";
        }

        determineWinner(); // Определяем победителя после завершения симуляции
    }

    void addCorporation(shared_ptr<Corporation> corporation) {
        corporations.push_back(corporation);
    }

    void determineWinner() const {
        shared_ptr<Planet> winner = nullptr;
        for (const auto& planet : planets) {
            if (!winner || planet->getTechnologyLevel() > winner->getTechnologyLevel() ||
                (planet->getTechnologyLevel() == winner->getTechnologyLevel() && planet->getMoney() > winner->getMoney())) {
                winner = planet;
            }
        }

        if (winner) {
            cout << "The winner is " << winner->getName() << " with technology level " << winner->getTechnologyLevel()
                 << " and money " << winner->getMoney() << ".\n";
        } else {
            cout << "No winner found.\n";
        }
    }
};



int main() {
    try {
        int planets, asteroids, routes;
        cout << fixed << setprecision(2);

        cout << "Enter the number of planets: ";
        cin >> planets;
        cout << "Enter the number of asteroids: ";
        cin >> asteroids;
        cout << "Enter the number of routes: ";
        cin >> routes;
        cout << "\n";

        shared_ptr<CoordinateGenerator> coordGen = make_shared<CoordinateGenerator>(
            make_shared<RealGenerator>(0.0, 500.0),
            make_shared<RealGenerator>(0.0, 500.0)
        );

        GameField field(planets, asteroids, routes, coordGen);
        field.generateField();

        // Добавление корпораций
        field.addCorporation(make_shared<LogisticsCompany>("Logistics Corp"));
        field.addCorporation(make_shared<TechTrader>("Tech Trader Corp"));
        field.addCorporation(make_shared<MinerCompany>("Miner Corp"));

        field.showInfo();
        field.runSimulation(40);
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
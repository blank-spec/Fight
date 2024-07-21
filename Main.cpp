#include <iostream>
#include <random>
#include <vector>
#include <thread>
#include <mutex>
using namespace std;

class RandomGenerator {
private:
	mt19937 gen;
	uniform_int_distribution<> dis;

public:
	RandomGenerator(int min, int max) : dis(min, max) {
		random_device rd;
		gen.seed(rd());
	}
	int getNumber() {
		return dis(gen);
	}
};

class Fighter {
private:
	int health;
	int damage;
	int armor;
	mutable mutex mtx;

public:
	Fighter(int health, int damage, int armor) :
		health(health), damage(damage), armor(armor) {}

	virtual void showInfo() const = 0;

	int getDamage() const { return damage; }
	int getHealth() const { return health; }

	void Damage(int harm) {
		lock_guard<mutex> guard(mtx);
		int currentHarn = harm - armor;
		if (currentHarn >= health) {
			health = 0;
		}
		else {
			health -= currentHarn;
		}
		
		if (health < 0) {
			health = 0;
		}
	}

	virtual ~Fighter() = default;
};

class Wildman : public Fighter {
public:
	Wildman(int health, int damage, int armor) :
		Fighter(health, damage, armor) {}

	void showInfo() const override {
		cout << "Widlman has " << getHealth() << " health and " << getDamage() << " damage" << endl;
	}
};

class Warrior : public Fighter {
public:
	Warrior(int health, int damage, int armor) :
		Fighter(health, damage, armor) {}

	void showInfo() const override {
		cout << "Warrior has " << getHealth() << " health and " << getDamage() << " damage" << endl;
	}
};

void fight(shared_ptr<Fighter> wildman, shared_ptr<Fighter> warrior, vector<string>& results, mutex& mtx, RandomGenerator& rg) {
	while (wildman->getHealth() > 0 && warrior->getHealth() > 0) {
		int number = rg.getNumber();

		if (number <= 50) { //You can change the chance
			warrior->Damage(wildman->getDamage());
		}
		else {
			wildman->Damage(warrior->getDamage());
		}
	}

	lock_guard<mutex> guard(mtx);
	if (wildman->getHealth() == 0 && warrior->getHealth() == 0) {
		results.push_back("Both died");
	}
	else if (wildman->getHealth() == 0) {
		results.push_back("Warrior won!");
	}
	else if (warrior->getHealth() == 0) {
		results.push_back("Wildman won!");
	}
}

int main() {
	shared_ptr<Fighter> wildman = make_shared<Wildman>(100, 30, 30); // You can change the characteristics
	shared_ptr<Fighter> warrior = make_shared<Warrior>(200, 20, 10);

	vector<string> results;
	vector<thread> threads;
	mutex mtx;
	RandomGenerator rg(1, 100);

	unsigned int countThreads = thread::hardware_concurrency();
	for (int i = 0; i < countThreads; ++i) {
		threads.emplace_back(fight, wildman, warrior, ref(results), ref(mtx), ref(rg));
	}

	for (auto& thr : threads) {
		thr.join();
	}

	for (const auto& rs : results) {
		cout << rs << endl;
	}

	return 0;
}

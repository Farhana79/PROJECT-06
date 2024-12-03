/**
* @date [12/02/2024]
 * @author [Farhana Sultana]
 */
#include "StationManager.hpp"
#include <iostream>

// Default Constructor
StationManager::StationManager() {
    // Initializes an empty station manager
}


// Adds a new station to the station manager
bool StationManager::addStation(KitchenStation* station) {
    return insert(item_count_, station);
}

// Removes a station from the station manager by name
bool StationManager::removeStation(const std::string& station_name) {
    for (int i = 0; i < item_count_; ++i) {
        if (getEntry(i)->getName() == station_name) {
            return remove(i);
        }
    }
    return false;
}

// Finds a station in the station manager by name
KitchenStation* StationManager::findStation(const std::string& station_name) const {
    Node<KitchenStation*>* searchptr = getHeadNode();
    while (searchptr != nullptr) {
        if (searchptr->getItem()->getName() == station_name) {
            return searchptr->getItem();
        }
        searchptr = searchptr->getNext();
    }
    return nullptr;
}

// Moves a specified station to the front of the station manager list
bool StationManager::moveStationToFront(const std::string& station_name) {
    // First, make sure the station exists
    if (findStation(station_name) == nullptr) {
        return false;
    }
    
    // If it's already at the front, return true
    if (getHeadNode()->getItem()->getName() == station_name) {
        return true;
    }

    Node<KitchenStation*>* searchptr = getHeadNode();
    while (searchptr != nullptr) {
        if (searchptr->getItem()->getName() == station_name) {
            // Make a copy of the station
            KitchenStation* station = searchptr->getItem();
            
            // Remove the station from its current position
            int pos = getStationIndex(searchptr->getItem()->getName());
            remove(pos);
            
            // Insert the station at the front
            insert(0, station);
            
            return true;  // Exit after moving the station
        }
        
        searchptr = searchptr->getNext();  // Move to the next node
    }
    
    return false;
}


int StationManager::getStationIndex(const std::string& name) const {
    Node<KitchenStation*>* searchptr = getHeadNode();
    int index = 0;
    while (searchptr != nullptr) {
        if (searchptr->getItem()->getName() == name) {
            return index;
        }
        searchptr = searchptr->getNext();
        index++;
    }
    return -1;
}

// Merges the dishes and ingredients of two specified stations
bool StationManager::mergeStations(const std::string& station_name1, const std::string& station_name2) {
    KitchenStation* station1 = findStation(station_name1);
    KitchenStation* station2 = findStation(station_name2);
    if (station1 && station2) {
        // take all the dishes from station2 and add them to station1
        for (Dish* dish : station2->getDishes()) {
            station1->assignDishToStation(dish);
        }
        // take all the ingredients from station2 and add them to station1
        for (Ingredient ingredient : station2->getIngredientsStock()) {
            station1->replenishStationIngredients(ingredient);
        }
        // remove station2 from the list
        removeStation(station_name2);
        return true;
    }
    return false;
}

// Assigns a dish to a specific station
bool StationManager::assignDishToStation(const std::string& station_name, Dish* dish) {
    KitchenStation* station = findStation(station_name);
    if (station) {
        return station->assignDishToStation(dish);
    }
    return false;
}

// Replenishes an ingredient at a specific station
bool StationManager::replenishIngredientAtStation(const std::string& station_name, const Ingredient& ingredient) {
    KitchenStation* station = findStation(station_name);
    if (station) {
        station->replenishStationIngredients(ingredient);
        return true;
    }
    return false;
}

// Checks if any station in the station manager can complete an order for a specific dish
bool StationManager::canCompleteOrder(const std::string& dish_name) const {
    Node<KitchenStation*>* searchptr = getHeadNode();
    while (searchptr != nullptr) {
        if (searchptr->getItem()->canCompleteOrder(dish_name)) {
            return true;
        }
        searchptr = searchptr->getNext();
    }
    return false;
}

// Prepares a dish at a specific station if possible
bool StationManager::prepareDishAtStation(const std::string& station_name, const std::string& dish_name) {
    KitchenStation* station = findStation(station_name);
    if (station && station->canCompleteOrder(dish_name)) {
        return station->prepareDish(dish_name);
    }
    return false;
}

std::queue<Dish *> StationManager::getDishQueue() const {
    return dish_queue_;
}

std::vector<Ingredient> StationManager::getBackupIngredients() const {
    return backup_ingredients_;
}

void StationManager::setDishQueue(std::queue<Dish *> dish_queue) {
    dish_queue_ = std::move(dish_queue);
}

void StationManager::addDishToQueue(Dish *dish) {
    if (dish != nullptr) {
        dish_queue_.push(dish);  // Add the dish to the queue
    }

}

void StationManager::addDishToQueue(Dish *dish, const Dish::DietaryRequest &request) {
    if (dish != nullptr){
        dish->dietaryAccommodations(request);
        dish_queue_.push(dish);
    }
}

bool StationManager::prepareNextDish() {
    if (!dish_queue_.empty()){
        Dish * dish =  dish_queue_.front();
        for (int i = 0; i < item_count_; ++i) {
            KitchenStation* station =  getEntry(i);
            if (station->canCompleteOrder(dish->getName())){
                station->prepareDish(dish->getName());
                dish_queue_.pop();
                return true;
            }
        }
    }
    return false;
}

void StationManager::displayDishQueue() const {
    // Create a copy of the queue to display without modifying the original queue
    std::queue<Dish *> temp_queue = getDishQueue();

    // Check if the queue is empty
    if (temp_queue.empty()) {
        std::cout << "The preparation queue is empty." << std::endl;
        return;
    }

    // Iterate through the queue and display the names of the dishes
    while (!temp_queue.empty()) {
        Dish *dish = temp_queue.front();  // Get the front dish
        if (dish != nullptr) {
            std::cout << dish->getName() << std::endl;  // Assume getName() returns the dish's name
        }
        temp_queue.pop();  // Remove the front dish from the temporary queue
    }
}

void StationManager::clearDishQueue() {
    while (!dish_queue_.empty()) {
        Dish *dish = dish_queue_.front();  // Get the front dish
        delete dish;                       // Free the memory allocated for the dish
        dish_queue_.pop();                 // Remove the dish from the queue
    }
}

bool StationManager::replenishStationIngredientFromBackup(const std::string &station_name,
                                                          const std::string &ingredient_name, int quantity) {

    for (auto it = backup_ingredients_.begin(); it != backup_ingredients_.end(); ++it) {
        if (it->name == ingredient_name) {
            if (it->quantity >= quantity) {
                // Deduct the quantity from the backup ingredient
                it->quantity -= quantity;

                // Create a copy of the ingredient with the replenished quantity
                Ingredient replenished_ingredient = *it;
                replenished_ingredient.quantity = quantity;

                // Replenish the ingredient at the station
                replenishIngredientAtStation(station_name, replenished_ingredient);

                // If the backup ingredient's quantity becomes zero, remove it from the list
                if (it->quantity == 0) {
                    backup_ingredients_.erase(it);
                }
                return true;  // Successfully replenished the ingredient
            } else {
                // Not enough quantity to replenish
                return false;
            }
        }
    }
    // Ingredient not found in backup
    return false;


}

bool StationManager::setBackupIngredients(const std::vector<Ingredient> &ingredients) {
    if (!ingredients.empty()) {
        backup_ingredients_ = ingredients;  // Replace the current backup stock with the new list
        return true;  // Successfully set the backup ingredients
    }
    return false;  // Return false if the provided list is empty
}

bool StationManager::addBackupIngredient(Ingredient ingredient) {
    for (auto &existing_ingredient : backup_ingredients_) {
        // If the ingredient already exists, increase its quantity
        if (existing_ingredient.name == ingredient.name) {
            existing_ingredient.quantity += ingredient.quantity;
            return true;  // Successfully updated existing ingredient
        }
    }

    // If the ingredient doesn't exist, add it to the backup stock
    backup_ingredients_.push_back(ingredient);
    return true;
}

void StationManager::clearBackupIngredients() {
    backup_ingredients_.clear();  // Clear all elements from the backup ingredients vector
}

void StationManager::processAllDishes() {
    std::queue<Dish *> notProcessed;

    while (!dish_queue_.empty()){
        Dish * dish =  dish_queue_.front();
        bool isDishPrepared = false;
        bool isDoshFound = false;
        std::cout <<  "PREPARING DISH: " << dish->getName() << std::endl;
        for (int i = 0; i < item_count_; ++i) {
            KitchenStation * kitchenStation = getEntry(i);
            std::cout << kitchenStation->getName() << ": attempting to prepare " << dish->getName() << "..." << std::endl;
            isDoshFound = false;
            //Check for the dish
            for (Dish * station_dish: kitchenStation->getDishes()) {

                if (station_dish->getName() == dish->getName()){
                    isDoshFound = true;
                    if (kitchenStation->canCompleteOrder(dish->getName())){
                        if (kitchenStation->prepareDish(dish->getName())){
                            isDishPrepared = true;
                            std::cout << kitchenStation->getName() << ": Successfully prepared " << dish->getName() << "." << std::endl;
                        }
                    }
                    else{
                        std::cout << kitchenStation->getName() << ": Insufficient ingredients. Replenishing ingredients..." << std::endl;

                        for (const Ingredient &requiredIngredient: station_dish->getIngredients()) {
                            bool isIngredientFound = false;
                            for (const Ingredient &stockIngredient: kitchenStation->getIngredientsStock()) {
                                if (requiredIngredient.name == stockIngredient.name){
                                    isIngredientFound = true;
                                    int requiredQuantity = stockIngredient.quantity  - requiredIngredient.required_quantity;
                                    if (requiredQuantity < 0){
                                        if (!replenishStationIngredientFromBackup(kitchenStation->getName(),requiredIngredient.name, (-1)*requiredQuantity)){
                                            std::cout << kitchenStation->getName() << ": Unable to replenish ingredients. Failed to prepare " << dish->getName() << "." << std::endl;
                                        }
                                    }
                                    break;
                                }
                            }
                            if (!isIngredientFound){
                                if (!replenishStationIngredientFromBackup(kitchenStation->getName(),requiredIngredient.name, requiredIngredient.required_quantity)){
                                    std::cout << kitchenStation->getName() << ": Unable to replenish ingredients. Failed to prepare " << dish->getName() << "." << std::endl;
                                    break;
                                }
                            }
                        }

                        if (kitchenStation->prepareDish(dish->getName())){
                            std::cout << kitchenStation->getName() << ": Ingredients replenished." << std::endl;
                            std::cout << kitchenStation->getName() << ": Successfully prepared " << dish->getName() << "." << std::endl;
                            isDishPrepared = true;
                            break;
                        }
                    }
                }
            }
            if (!isDoshFound){
                std::cout << kitchenStation->getName() <<  ": Dish not available. Moving to next station..." << std:: endl;
            }
            if (isDishPrepared){
                break;
            }
        }
        if (!isDishPrepared){
            std::cout <<  dish->getName() <<" was not prepared." << std::endl;
            notProcessed.push(dish);
        }
        dish_queue_.pop();
        std::cout << std::endl;
    }
    setDishQueue(notProcessed);
    std::cout<<std::endl<<std::endl<< "All dishes have been processed." << std::endl;
}




#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <random>
#include <fstream>
#include <sstream>
#include <map>

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

class DoubleGenerator : public NumberGenerator {
protected:
    uniform_real_distribution<> dis;

public:
    DoubleGenerator(double min, double max) : dis(min, max) {}
    double generate() override { return dis(gen); }
};

class PaymentStrategy {
public:
    virtual bool pay(double amount) = 0; 
    virtual string get_payment_method() const = 0;  
    virtual ~PaymentStrategy() = default;
};

class CashPayment : public PaymentStrategy {
public:
    bool pay(double amount) override {
        cout << "Paid " << amount << " using Cash." << endl;
        return true;  
    }

    string get_payment_method() const override {
        return "Cash";
    }
};

class CardPayment : public PaymentStrategy {
public:
    bool pay(double amount) override {
        cout << "Paid " << amount << " using Card." << endl;
        return true;  
    }

    string get_payment_method() const override {
        return "Card";
    }
};

class CryptoPayment : public PaymentStrategy {
public:
    bool pay(double amount) override {
        cout << "Paid " << amount << " using Cryptocurrency." << endl;
        return true;  
    }

    string get_payment_method() const override {
        return "Cryptocurrency";
    }
};

class Seller {
protected:
    string name;
    int id;

public:
    Seller(const string& name, int id) : name(name), id(id) {}

    string get_name() const { return name; }
    int get_id() const { return id; }
};

class Product {
protected:
    string name;
    double price;
    int count;
    string seller_name;

public:
    Product(const string& product_name, double product_price, int product_count, const Seller& seller)
        : name(product_name), price(product_price), count(product_count), seller_name(seller.get_name()) {}

    void display() const {
        cout << "Product: " << name << ", Price: " << price
             << ", Count: " << count << ", Seller: " << seller_name << endl;
    }

    string get_name() const {return name;}  
    double get_price() const { return price; }

    int get_count() const { return count; }
    void decrease_count() { count--; }
};

class Customer {
    string name; // Add a name to identify customers
    int id;
    double cash_balance;
    double card_balance;
    double crypto_balance;
    shared_ptr<PaymentStrategy> payment_strategy;
    int purchase_count; // Счетчик покупок

public:
    Customer(string customer_name, int customer_id, double initial_balance)
        : name(customer_name), id(customer_id), cash_balance(initial_balance), card_balance(initial_balance), crypto_balance(0.015), purchase_count(0) {}

    string get_name() const { return name; }  // Get customer name
    int get_id() const { return id; }

    double get_cash_balance() const { return cash_balance; }
    double get_card_balance() const { return card_balance; }
    double get_crypto_balance() const { return crypto_balance; }

    void decrease_balance(double amount, const string& method) {
        if (method == "Cash") cash_balance -= amount;
        else if (method == "Card") card_balance -= amount;
        else if (method == "Cryptocurrency") crypto_balance -= amount;
    }

    void set_payment_strategy(shared_ptr<PaymentStrategy> strategy) {
        payment_strategy = strategy;
    }

    void set_card_balance(double amount) {
        card_balance = amount;
    }

    void set_crypto_balance(double amount) {
        crypto_balance = amount;
    }

    void print_receipt(const Product& product) const {
        cout << "Receipt for " << name << ": " << product.get_name() << " purchased for " << product.get_price() << endl;
    }

    bool purchase_product(Product& product) {
        double available_balance = 0;
        if (payment_strategy->get_payment_method() == "Cash") {
            available_balance = cash_balance;
        } else if (payment_strategy->get_payment_method() == "Card") {
            available_balance = card_balance;
        } else if (payment_strategy->get_payment_method() == "Cryptocurrency") {
            available_balance = crypto_balance;
        }

        if (available_balance >= product.get_price() && product.get_count() > 0) {
            if (payment_strategy->pay(product.get_price())) {
                decrease_balance(product.get_price(), payment_strategy->get_payment_method());
                product.decrease_count();
                print_receipt(product);  // Print receipt
                generate_receipt_file(product);  // Generate receipt file
                purchase_count++; // Увеличиваем счетчик покупок
                return true;
            }
        } else {
            cout << "Insufficient balance or product is out of stock!" << endl;
            return false;
        }
    }

    void generate_receipt_file(const Product& product) const {
        string filename = "receipt_" + name + "_" + to_string(id) + "_" + to_string(purchase_count) + ".txt";
        ofstream file(filename);
        if (file.is_open()) {
            file << "Receipt for " << name << " (ID: " << id << ")\n";
            file << "Product: " << product.get_name() << "\n";
            file << "Price: " << product.get_price() << "\n";
            file << "Payment Method: " << payment_strategy->get_payment_method() << "\n";
            file << "Remaining Balance:\n";
            file << "Cash: " << cash_balance << "\n";
            file << "Card: " << card_balance << "\n";
            file << "Cryptocurrency: " << crypto_balance << "\n";
            file.close();
            cout << "Receipt generated: " << filename << endl;
        } else {
            cout << "Unable to create receipt file." << endl;
        }
    }

    // Display customer balance
    void display_balance() const {
        cout << "Customer " << name << "'s Balance - Cash: " << cash_balance << ", Card: " << card_balance << ", Crypto: " << crypto_balance << endl;
    }
};

class Marketplace {
    vector<shared_ptr<Product>> products;
    vector<shared_ptr<Seller>> sellers;
    vector<shared_ptr<Customer>> customers;

public:
    void add_product(const string& product_name, double price, int count, const Seller& seller) {
        products.push_back(make_shared<Product>(product_name, price, count, seller));
    }

    void add_seller(const Seller& seller) {
        sellers.push_back(make_shared<Seller>(seller));
    }
    
    vector<shared_ptr<Customer>> get_customers() const {
        return customers; 
    }

    vector<shared_ptr<Seller>> get_sellers() const{
        return sellers;
    }

    shared_ptr<Seller> get_seller_by_id(int id) const {
        for (const auto& seller : sellers) {
            if (seller->get_id() == id) {
                return seller;  
            }
        }
        return nullptr; 
    }

    void add_customer(const string& name, int id, double initial_balance) {
        customers.push_back(make_shared<Customer>(name, id, initial_balance));
    }

    void display_customers() const {
        if (customers.empty()) {
            cout << "No customers available!" << endl;
            return;
        } else {
            for (const auto& cust : customers) {
                cout << "Customer: " << cust->get_name() << " (ID: " << cust->get_id() << ")\n";
                cust->display_balance();
            }
        }
    }

    void remove_customer(const shared_ptr<Customer>& customer) {
        customers.erase(remove(customers.begin(), customers.end(), customer), customers.end());
        cout << "Customer " << customer->get_name() << " has been removed due to insufficient funds!" << endl;
    }

    void display_products() const {
        if (products.empty()) {
            cout << "No products available!" << endl;
            return;
        }
        for (const auto& product : products) {
            product->display();
        }
    }

    shared_ptr<Product> get_product(int index) const {
        if (index >= 0 && index < products.size()) {
            return products[index];
        }
        return nullptr;
    }

    vector<shared_ptr<Product>> get_products() const {
        return products;
    }

    void display_sellers() const {
        if (sellers.empty()) {
            cout << "No sellers available!" << endl;
            return;
        } else {
            for (const auto& seller : sellers) {
                cout << "Seller: " << seller->get_name() << " (ID: " << seller->get_id() << ")\n";
            }
        }
    }
};

bool isNumber(const string& str) {
    if (str.empty()) return false;

    for (char ch : str) {
        if (!isdigit(ch)) return false;  
    }

    return true;
}

void add_product(Marketplace& marketplace) {
    if (marketplace.get_sellers().empty()) {
        cout << "No sellers available!" << endl;
        return; 
    }

    marketplace.display_sellers();
    cout << "Enter seller's ID to add product: ";
    int seller_id;
    cin >> seller_id;
    cin.ignore();  // Ignore leftover newline character

    string name, product_name;
    double price;
    int count;
    cout << "Enter product name: ";
    getline(cin, product_name);
    cout << "Enter price: ";
    cin >> price;
    cout << "Enter stock count: ";
    cin >> count;
    cin.ignore();  // Ignore leftover newline character
    
    bool seller_found = false;
    for (const auto& seller : marketplace.get_sellers()) {
        if (seller->get_id() == seller_id) {
            marketplace.add_product(product_name, price, count, *seller);
            seller_found = true;
            break;
        }
    }

    if (!seller_found) {
        cout << "Seller not found!" << endl;
    }
}

void adds_seller(Marketplace& marketplace) {
    string name;
    int id;
    cout << "Enter seller name: ";
    getline(cin, name);
    cout << "Enter his/her id: ";
    cin >> id;
    cin.ignore(); 

    if(name.empty()){
        cout << "Error: Name cannot be empty!" << endl;
        return;
    }
    if(id < 0){
        cout << "Error: ID cannot be < 0!!!" << endl;
        return;
    }

    for(const auto& sel : marketplace.get_sellers()){
        if(sel -> get_id() == id){
            cout << "Error: Seller with ID " << id << " already exists!" << endl;
            return;  
        }
    }

    Seller seller(name, id);
    marketplace.add_seller(seller);
    cout << "Seller added successfully!" << endl;
}

void add_customer(Marketplace& marketplace) {
    string name;
    int id;
    cout << "Enter customer name: ";
    getline(cin, name);
    cout << "Enter customer id: ";
    cin >> id;
    cin.ignore(); // игнорируем символ новой строки

    if (name.empty()) {
        cout << "Error: Name cannot be empty!" << endl;
        return;
    }
    if (id < 0) {
        cout << "Error: ID cannot be negative!" << endl;
        return;
    }

    for (const auto& cust : marketplace.get_customers()) {
        if (cust->get_id() == id) {
            cout << "Error: Customer with ID " << id << " already exists!" << endl;
            return;
        }
    }

    IntGenerator for_nalichka_with_card(0, 10000);
    DoubleGenerator for_crypto(0, 0.015);

    double cash_balance = for_nalichka_with_card.generate();
    double card_balance = for_nalichka_with_card.generate();
    double crypto_balance = for_crypto.generate();

    shared_ptr<Customer> customer = make_shared<Customer>(name, id, cash_balance);
    customer->set_card_balance(card_balance);
    customer->set_crypto_balance(crypto_balance);
    customer->display_balance();

    marketplace.add_customer(name, id, cash_balance);
}

void purchase_product(Marketplace& marketplace) {
    if (marketplace.get_customers().empty()) {
        cout << "No customers available!" << endl;
        return;
    }

    if (marketplace.get_products().empty()) {
        cout << "No products available!" << endl;
        return;
    }

    marketplace.display_customers();
    cout << "Select a customer to purchase product (enter index): ";
    int customer_index;
    cin >> customer_index;
    cin.ignore();

    if (customer_index < 1 || customer_index > marketplace.get_customers().size()) {
        cout << "Invalid customer index!" << endl;
        return;
    }

    shared_ptr<Customer> selected_customer = marketplace.get_customers()[customer_index - 1];

    marketplace.display_products();
    cout << "Enter the index of the product to buy: ";
    int product_index;
    cin >> product_index;
    cin.ignore();

    if (product_index < 1 || product_index > marketplace.get_products().size()) {
        cout << "Invalid product index!" << endl;
        return;
    }

    shared_ptr<Product> product = marketplace.get_products()[product_index - 1];

    cout << "Choose payment method: \n";
    cout << "1. Cash\n";
    cout << "2. Card\n";
    cout << "3. Cryptocurrency\n";
    int payment_choice;
    cin >> payment_choice;
    cin.ignore();

    shared_ptr<PaymentStrategy> payment_strategy = nullptr;
    switch (payment_choice) {
        case 1:
            if (selected_customer->get_cash_balance() >= product->get_price()) {
                payment_strategy = make_shared<CashPayment>();
            } else {
                cout << "Insufficient cash balance!" << endl;
            }
            break;
        case 2:
            if (selected_customer->get_card_balance() >= product->get_price()) {
                payment_strategy = make_shared<CardPayment>();
            } else {
                cout << "Insufficient card balance!" << endl;
            }
            break;
        case 3:
            if (selected_customer->get_crypto_balance() >= product->get_price()) {
                payment_strategy = make_shared<CryptoPayment>();
            } else {
                cout << "Insufficient cryptocurrency balance!" << endl;
            }
            break;
        default:
            cout << "Invalid payment method choice!" << endl;
            return;
    }

    if (payment_strategy) {
        selected_customer->set_payment_strategy(payment_strategy);
        if (selected_customer->purchase_product(*product)) {
            cout << "Purchase successful!" << endl;
        } else {
            // Remove customer if insufficient funds
            marketplace.remove_customer(selected_customer);
        }
    }
}

void menu(Marketplace& marketplace) {
    cout << ">========== MENU ==========<" << "\n";
    cout << "1. Add new product" << "\n";
    cout << "2. Add new seller" << "\n";
    cout << "3. Add new customer" << "\n";
    cout << "4. Display products" << "\n";
    cout << "5. Display sellers" << "\n";
    cout << "6. Display customers" << "\n";
    cout << "7. Purchase product" << "\n";
    cout << "8. Exit" << "\n";
    cout << ">==========================<" << "\n";

    int choice = 0;
    vector<shared_ptr<Customer>> customers;

    while(true) {
        cout << "Your choice: ";
        string input;
        getline(cin, input);

        if (!isNumber(input)) {
            cout << "Invalid input! Please enter a number." << endl;
            continue;
        }

        choice = stoi(input);
        if (choice == 8) {
            cout << "Exiting..." << endl;
            break;
        }

        switch(choice) {
            case 1:
                add_product(marketplace);
                break;
            case 2:
                adds_seller(marketplace);
                break;
            case 3:
                add_customer(marketplace);
                break;
            case 4:
                marketplace.display_products();
                break;
            case 5:
                marketplace.display_sellers();
                break;
            case 6:
                marketplace.display_customers();
                break;
            case 7: {
                // Покупка товара
                purchase_product(marketplace);
                break;
            }
            default:
                cout << "Invalid choice! Please try again." << "\n";
        }
    }
}

int main() {
    Marketplace marketplace;
    menu(marketplace);

    return 0;
}
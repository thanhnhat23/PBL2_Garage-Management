#include "../Class/Table.h"

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <unordered_map>
#include <sstream>

using namespace std;

namespace {
    string lcase(string s) {
        transform(s.begin(), s.end(), s.begin(),
                [](unsigned char c){ return (char)tolower(c); });
        return s;
    }
    bool endsWithNoCase(const string& s, const string& suf) {
        if (s.size() < suf.size()) return false;
        for (size_t i=0;i<suf.size();++i)
            if ((char)tolower(s[s.size()-suf.size()+i]) != (char)tolower(suf[i])) return false;
        return true;
    }
    string ensureSuffix(const string& s, const string& suf) {
        return endsWithNoCase(s, suf) ? s : (s + suf);
    }

    struct Table {
        vector<string> headers;
        vector<vector<string>> rows;
        vector<bool> rightAlign;

        void print() const {
            vector<size_t> W(headers.size(), 0);
            for (size_t c=0;c<headers.size();++c) W[c] = headers[c].size();
            for (auto& r : rows)
                for (size_t c=0;c<r.size();++c) W[c] = max(W[c], r[c].size());

            cout << "+";
            for (size_t c=0;c<W.size();++c) cout << string(W[c]+2, '-') << "+";
            cout << "\n|";
            for (size_t c=0;c<W.size();++c)
                cout << " " << left << setw((int)W[c]) << headers[c] << " |";
            cout << "\n+";
            for (size_t c=0;c<W.size();++c) cout << string(W[c]+2, '-') << "+";
            cout << "\n";
            for (auto& r : rows) {
                cout << "|";
                for (size_t c=0;c<W.size();++c) {
                    ios old(nullptr); old.copyfmt(cout);
                    if (c < rightAlign.size() && rightAlign[c]) cout << right;
                    else cout << left;
                    cout << " " << setw((int)W[c]) << (c<r.size()?r[c]:"") << " ";
                    cout.copyfmt(old);
                    cout << "|";
                }
                cout << "\n";
            }
            cout << "+";
            for (size_t c=0;c<W.size();++c) cout << string(W[c]+2, '-') << "+";
            cout << "\n";
        }
    };

}

namespace ui {

    void DrawPanel(const string& title) {
        system("cls");
        int W = max(36, (int)title.size() + 6);
        cout << "+" << string(W-2, '-') << "+\n";
        int padL = (W-2 - (int)title.size())/2;
        cout << "|" << string(padL, ' ') << title
            << string(W-2 - padL - (int)title.size(), ' ') << "|\n";
        cout << "+" << string(W-2, '-') << "+\n\n";
    }

    // -------- BRAND --------
    void PrintBrands(const vector<Brand>& brands) {
        Table t;
        t.headers = {"ID", "Brand"};
        t.rightAlign = {false, false};
        for (auto& b : brands) {
            t.rows.push_back({ b.getId(), b.getName() }); // Brand getters
        }
        t.print();
    }

    // -------- ROUTES --------
    void PrintRoutes(const vector<Route>& routes) {
        Table t;
        t.headers = {"ID", "Route", "Distance", "Minutes"};
        t.rightAlign = {false, false, true, true};

        for (auto& r : routes) {
            string name = r.getName().empty() ? (r.getStart() + " - " + r.getEnd()) : r.getName(); // :contentReference[oaicite:4]{index=4}
            string dist = ensureSuffix(r.getDistance(), "km");   // :contentReference[oaicite:5]{index=5}
            string mins = ensureSuffix(r.getDuration(), "p");    // :contentReference[oaicite:6]{index=6}
            t.rows.push_back({ r.getId(), name, dist, mins });   // :contentReference[oaicite:7]{index=7}
        }
        t.print();
    }

    // -------- BUSES --------
    void PrintBuses(const vector<Bus>& buses) {
        Table t;
        t.headers    = {"ID", "Plate", "Type", "Brand"};
        t.rightAlign = {false, false, false, false};
        for (auto& b : buses) {
            t.rows.push_back({ b.getId(), b.getName(), b.getType(), b.getBrandId() }); // :contentReference[oaicite:8]{index=8}
        }
        t.print();
    }

    // -------- DRIVERS --------
    void PrintDrivers(const vector<Driver>& drivers) {
        Table t;
        t.headers    = {"ID", "Name", "Phone", "Exp(yrs)", "Bus"};
        t.rightAlign = {false, false, false, true, false};
        for (auto& d : drivers) {
            t.rows.push_back({ d.getId(), d.getName(), d.getPhone(), to_string(d.getExp()), d.getBus() }); // :contentReference[oaicite:9]{index=9}
        }
        t.print();
    }

    // -------- SEATS --------
    void PrintSeats(const vector<Seat>& seats) {
        Table t;
        t.headers    = {"Seat", "Status"};
        t.rightAlign = {true,  false};
        for (auto& s : seats) {
            t.rows.push_back({ to_string(s.getSeatNo()), (s.getStatus() ? "Full" : "Empty") }); // :contentReference[oaicite:10]{index=10}
        }
        t.print();
    }

    // -------- TICKETS --------
    void PrintTickets(const vector<Ticket>& tickets,
                    const vector<Trip>& trips,
                    const vector<Route>& routes,
                    const vector<Bus>& buses) {
        unordered_map<string,const Trip*>  mTrip;
        unordered_map<string,const Route*> mRoute;
        unordered_map<string,const Bus*>   mBus;

        for (auto& t : trips)  mTrip[t.getId()]   = &t;     // :contentReference[oaicite:11]{index=11}
        for (auto& r : routes) mRoute[r.getId()]  = &r;     // :contentReference[oaicite:12]{index=12}
        for (auto& b : buses)  mBus[b.getId()]    = &b;     // :contentReference[oaicite:13]{index=13}

        Table t;
        t.headers    = {"TID","Seat","Type","Passenger","Phone","Price","BookedAt","Payment","Route","Trip"};
        t.rightAlign = {false,true,false,false,false,true,false,false,false,false};

        for (auto& tk : tickets) {
            const Trip*  tr = nullptr;
            const Route* ro = nullptr;
            const Bus*   bu = nullptr;

            auto itT = mTrip.find(tk.getTripId());
            if (itT != mTrip.end()) tr = itT->second;
            if (tr) {
                auto itR = mRoute.find(tr->getRouteId());
                if (itR != mRoute.end()) ro = itR->second;
            }
            auto itB = mBus.find(tk.getBusId());
            if (itB != mBus.end()) bu = itB->second;

            string routeName = (ro ? (ro->getStart() + " - " + ro->getEnd()) : "");          // :contentReference[oaicite:14]{index=14}
            string tripTime  = (tr ? (tr->getDepart() + " - " + tr->getArrival()) : "");     // :contentReference[oaicite:15]{index=15}
            string type      = (bu ? bu->getType() : "");                                     // :contentReference[oaicite:16]{index=16}
            string priceStr  = to_string(tk.getPrice()) + "vnd";

            t.rows.push_back({
                tk.getId(), to_string(tk.getSeatNo()), type,
                tk.getPassengerName(), tk.getPhoneNumber(),
                priceStr, tk.getBookedAt(), tk.getPaymentMethod(),
                routeName, tripTime
            });
        }
        t.print();
    }

}

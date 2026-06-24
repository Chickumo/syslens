#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netpacket/packet.h>
#include <map>
#include <fstream>
#include <sstream>

using namespace std;

const string separator = "====================================================================";

struct NetworkInterface {
    string ipv4;
    string mac;
};

double getUsedRamGB(double& totalRamGB) {
    ifstream meminfo("/proc/meminfo");

    string key;
    long long value;
    string unit;

    long long memTotal = 0;
    long long memAvailable = 0;

    while (meminfo >> key >> value >> unit) {
        if (key == "MemTotal:")
            memTotal = value;
        else if (key == "MemAvailable:")
            memAvailable = value;

        if (memTotal && memAvailable)
            break;
    }

    totalRamGB = memTotal / (1024.0 * 1024.0);

    return (memTotal - memAvailable) / (1024.0 * 1024.0);
}

void systemInfo() {
    struct sysinfo si;
    struct utsname u;
    struct ifaddrs* ifaddr;
    char hostname[256];

    if (sysinfo(&si) == 0) {cout << right << setw(separator.length()) << "\n                                                  Uptime: " << si.uptime / 3600 << "H " << (si.uptime % 3600) / 60 << "M " << si.uptime % 60 << "S" << endl;}
    cout << separator << endl;
    if (uname(&u) == 0) {cout << "\nOS: " << u.sysname << " " << u.release << " " << u.machine << endl;}
    if (gethostname(hostname, sizeof(hostname)) == 0) {cout << "Hostname : " << hostname << endl;}

    map<string, NetworkInterface> interfaces;

    if (getifaddrs(&ifaddr) == 0) {
        for (auto* p = ifaddr; p; p = p->ifa_next) {
            if (!p->ifa_addr)
                continue;

            string name = p->ifa_name;

            if (p->ifa_addr->sa_family == AF_INET) {
                char ip[INET_ADDRSTRLEN];

                auto* addr =
                    (struct sockaddr_in*)p->ifa_addr;

                inet_ntop(AF_INET, &(addr->sin_addr), ip, sizeof(ip));

                interfaces[name].ipv4 = ip;
            }

            else if (p->ifa_addr->sa_family == AF_PACKET) {
                auto* s =
                    (struct sockaddr_ll*)p->ifa_addr;
                if (s->sll_halen > 0) {
                    stringstream mac;
                    for (int i = 0; i < s->sll_halen; i++) {
                        if (i)
                            mac << ":";
                        mac << hex
                            << setw(2)
                            << setfill('0')
                            << (int)s->sll_addr[i];
                    }

                    interfaces[name].mac = mac.str();
                }
            }
        }

        freeifaddrs(ifaddr);
    }

    string gateway = "Unknown";

    ifstream route("/proc/net/route");

    if (route) {

        string line;
        getline(route, line);

        while (getline(route, line)) {

            istringstream iss(line);

            string iface;
            string destination;
            string gw;

            iss >> iface >> destination >> gw;

            if (destination == "00000000") {

                unsigned long g = stoul(gw, nullptr, 16);
                struct in_addr addr;
                addr.s_addr = g;
                gateway = inet_ntoa(addr);
                break;

            }
        }
    }

    cout << "\nINTERFACES\n";

    for (const auto& [name, info] : interfaces) {

        if (name == "lo")
            continue;

        cout << "  " << name << '\n';

        if (!info.ipv4.empty()){cout << "    IPv4 : " << info.ipv4 << '\n';}
        if (!info.mac.empty()){cout << "    MAC  : " << info.mac << '\n';}
        cout << '\n';
    }

    cout << "GATEWAY - " << gateway << endl;

    double totalRam;
    double usedRam = getUsedRamGB(totalRam);

    cout << "\nRAM: " << fixed << setprecision(2) << usedRam << " GB / " << totalRam << " GB" << endl;
}

void displayMenu() {

    cout << "\033[1;36m";

    cout << R"( ███████╗██╗   ██╗███████╗██╗     ███████╗███╗   ██╗███████╗
 ██╔════╝╚██╗ ██╔╝██╔════╝██║     ██╔════╝████╗  ██║██╔════╝
 ███████╗ ╚████╔╝ ███████╗██║     █████╗  ██╔██╗ ██║███████╗
 ╚════██║  ╚██╔╝  ╚════██║██║     ██╔══╝  ██║╚██╗██║╚════██║
 ███████║   ██║   ███████║███████╗███████╗██║ ╚████║███████║
 ╚══════╝   ╚═╝   ╚══════╝╚══════╝╚══════╝╚═╝  ╚═══╝╚══════╝
)" << endl;

    cout << "\033[0m";

    string info =
        "PID: " + to_string(getpid()) +
        "\tPPID: " + to_string(getppid()) +
        "\tShell: " + getenv("SHELL") +
        "\tUser: " + getenv("USER");

    cout << left
         << setw(separator.length())
         << info
         << endl;

    systemInfo();
}

int main() {
    displayMenu();
    return 0;
}
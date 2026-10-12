// The system report's parsing and formatting (app/sysinfo.h).

#include "common.h"

#include "app/sysinfo.h"

void test_sysinfo_parse()
{
    using namespace th::sysinfo;
    CHECK(parse_os_release("NAME=\"Ubuntu\"\nVERSION=\"24.04 LTS\"\nPRETTY_NAME=\"Ubuntu 24.04.1 LTS\"\nID=ubuntu\n") ==
          "Ubuntu 24.04.1 LTS");
    CHECK(parse_os_release("NAME=Alpine\nVERSION='3.20'\n") == "Alpine 3.20");
    CHECK(parse_os_release("NAME=Void\n") == "Void");
    CHECK(parse_os_release("ID=x\n").empty());
    CHECK(parse_os_release("PRETTY_NAME=\"CRLF Linux\"\r\nID=x\r\n") == "CRLF Linux");
    CHECK(parse_os_release("").empty());

    CHECK(parse_cpuinfo_model("processor\t: 0\nvendor_id\t: GenuineIntel\nmodel name\t: Intel(R) Core(TM) i7-8550U\n"
                              "model name\t: second\n") == "Intel(R) Core(TM) i7-8550U");
    CHECK(parse_cpuinfo_model("processor\t: 0\nHardware\t: BCM2835\nModel\t\t: Raspberry Pi 4\n") == "BCM2835");
    CHECK(parse_cpuinfo_model("processor\t: 0\nBogoMIPS\t: 50.00\n").empty());

    CHECK(format_mib(512) == "512 MiB");
    CHECK(format_mib(16384) == "16 GiB");
    CHECK(format_mib(7782) == "7.6 GiB");

    CHECK(without_home("/home/ana/.local/share/ThemeHospitalEnhanced/game", "/home/ana/") ==
          "~/.local/share/ThemeHospitalEnhanced/game");
    CHECK(without_home("/home/ana", "/home/ana") == "~");
    CHECK(without_home("/home/anabel/game", "/home/ana") == "/home/anabel/game");
    CHECK(without_home("C:\\Users\\Ana\\AppData\\Local\\x", "C:\\Users\\Ana\\") == "~\\AppData\\Local\\x");
    CHECK(without_home("/persist/game", "/") == "/persist/game");
    CHECK(without_home("/opt/th/game", "") == "/opt/th/game");
}

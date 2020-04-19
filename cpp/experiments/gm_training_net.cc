#include <string>
#include "cpp/networks/controller.cc"
#include "cpp/networks/gm.cc"

using namespace std;
using namespace controller;
using namespace algorithms::gm;

int main(int argc, char **argv) {

    string cfg = string(argv[1]);

    Controller<GmNet> ctrl(cfg);
    ctrl.InitializeSimulation();
    ctrl.ShowNetworkInfo();
    ctrl.TrainOverNetwork();

    return 0;
}
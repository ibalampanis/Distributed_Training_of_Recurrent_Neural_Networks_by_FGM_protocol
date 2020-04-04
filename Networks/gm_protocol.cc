#include <jsoncpp/json/json.h>
#include "gm_protocol.hh"
#include "RNN_models/predictor/RNNPredictor.hh"
#include "dds/dsarch.hh"
//#include "dds/dds.hh"

using namespace gm_protocol;

/*********************************************
	model_state & p_model_state
*********************************************/

size_t model_state::byte_size() const {
    size_t num_of_params = 0;
    for (arma::mat param:_model) {
        num_of_params += param.n_elem;
    }
    return num_of_params * sizeof(float);
}

size_t p_model_state::byte_size() const {
    size_t num_of_params = 0;
    for (arma::mat *param:_model) {
        num_of_params += param->n_elem;
    }
    return num_of_params * sizeof(float);
}

size_t int_num::byte_size() const {
    return sizeof(size_t);
}

size_t matrix_message::byte_size() const {
    return sizeof(float) * sub_params.n_elem;
}

/*********************************************
	ml_safezone_function
*********************************************/

ml_safezone_function::ml_safezone_function(vector<arma::mat> &mdl) : GlobalModel(mdl) {}

void ml_safezone_function::updateDrift(vector <arma::mat> &drift, vector<arma::mat *> &vars, float mul) const {
    drift.clear();
    for (size_t i = 0; i < GlobalModel.size(); i++) {
        arma::mat dr = arma::mat(arma::size(*vars.at(i)), arma::fill::zeros);
        dr = mul * (*vars.at(i) - GlobalModel.at(i));
        drift.push_back(dr);
    }
}

ml_safezone_function::~ml_safezone_function() {}


/*********************************************
	Batch_Learning
*********************************************/

Batch_Learning::Batch_Learning(vector <arma::mat> &GlMd) : ml_safezone_function(GlMd), threshold(32) {
    hyperparameters.push_back(32.);
}

Batch_Learning::Batch_Learning(vector <arma::mat> &GlMd, size_t thr) : ml_safezone_function(GlMd), threshold(thr) {
    hyperparameters.push_back(thr);
}

size_t Batch_Learning::checkIfAdmissible(const size_t counter) const {
    size_t sz = threshold - counter;
    return sz;
}

size_t Batch_Learning::byte_size() const {
    size_t num_of_params = 0;
    for (arma::mat param:GlobalModel) {
        num_of_params += param.n_elem;
    }
    return num_of_params * sizeof(float) + sizeof(size_t);
}

Batch_Learning::~Batch_Learning() {}


/*********************************************
	Variance_safezone_func
*********************************************/

Variance_safezone_func::Variance_safezone_func(vector<arma::mat> &GlMd) : ml_safezone_function(GlMd), threshold(1.),
                                                                          batch_size(32) {
    hyperparameters.push_back(1.);
    hyperparameters.push_back(32.);
}

Variance_safezone_func::Variance_safezone_func(vector<arma::mat> &GlMd, size_t batch_sz) : ml_safezone_function(GlMd),
                                                                                           threshold(1.),
                                                                                           batch_size(batch_sz) {
    hyperparameters.push_back(1.);
    hyperparameters.push_back(batch_sz);
}

Variance_safezone_func::Variance_safezone_func(vector<arma::mat> &GlMd, float thr) : ml_safezone_function(GlMd),
                                                                                     threshold(thr), batch_size(32) {
    hyperparameters.push_back(thr);
    hyperparameters.push_back(32.);
}

Variance_safezone_func::Variance_safezone_func(vector<arma::mat> &GlMd, float thr, size_t batch_sz)
        : ml_safezone_function(GlMd), threshold(thr), batch_size(batch_sz) {
    hyperparameters.push_back(thr);
    hyperparameters.push_back(batch_sz);
}

float Variance_safezone_func::Zeta(const vector <arma::mat> &mdl) const {
    float res = 0.;
    for (size_t i = 0; i < mdl.size(); i++) {
        arma::mat subtr = GlobalModel.at(i) - mdl.at(i);
        res += arma::dot(subtr, subtr);
    }
    return std::sqrt(threshold) - std::sqrt(res);
}

float Variance_safezone_func::Zeta(const vector<arma::mat *> &mdl) const {
    float res = 0.;
    for (size_t i = 0; i < mdl.size(); i++) {
        arma::mat subtr = GlobalModel.at(i) - *mdl.at(i);
        res += arma::dot(subtr, subtr);
    }
    return std::sqrt(threshold) - std::sqrt(res);
}

size_t Variance_safezone_func::checkIfAdmissible(const size_t counter) const {
    return batch_size - counter;
}

float Variance_safezone_func::checkIfAdmissible(const vector <arma::mat> &mdl) const {
    float var = 0.;
    for (size_t i = 0; i < mdl.size(); i++) {
        arma::mat sub = mdl.at(i) - GlobalModel.at(i);
        var += arma::dot(sub, sub);
    }
    return threshold - var;
}

float Variance_safezone_func::checkIfAdmissible(const vector<arma::mat *> &mdl) const {
    float var = 0.;
    for (size_t i = 0; i < mdl.size(); i++) {
        arma::mat sub = *mdl.at(i) - GlobalModel.at(i);
        var += arma::dot(sub, sub);
    }
    return threshold - var;
}

float Variance_safezone_func::checkIfAdmissible(const vector<arma::mat *> &par1, const vector <arma::mat> &par2) const {
    float res = 0.;
    for (size_t i = 0; i < par1.size(); i++) {
        arma::mat subtr = *par1.at(i) - par2.at(i);
        res += arma::dot(subtr, subtr);
    }
    return std::sqrt(threshold) - std::sqrt(res);
}

float Variance_safezone_func::checkIfAdmissible_reb(const vector<arma::mat *> &par1, const vector<arma::mat> &par2,
                                                    float coef) const {
    float res = 0.;
    for (size_t i = 0; i < par1.size(); i++) {
        arma::mat subtr = *par1.at(i) - par2.at(i);
        ////subtr *= 2.;
        subtr *= coef;
        res += arma::dot(subtr, subtr);
    }
    ////return 0.5*(std::sqrt(threshold)-std::sqrt(res));
    return coef * (std::sqrt(threshold) - std::sqrt(res));
}

float Variance_safezone_func::checkIfAdmissible_v2(const vector <arma::mat> &drift) const {
    float var = 0.;
    for (size_t i = 0; i < drift.size(); i++) {
        var += arma::dot(drift.at(i), drift.at(i));
    }
    return threshold - var;
}

float Variance_safezone_func::checkIfAdmissible_v2(const vector<arma::mat *> &drift) const {
    float var = 0.;
    for (size_t i = 0; i < drift.size(); i++) {
        var += arma::dot(*drift.at(i), *drift.at(i));
    }
    return threshold - var;
}

size_t Variance_safezone_func::byte_size() const {
    size_t num_of_params = 0;
    for (arma::mat param:GlobalModel) {
        num_of_params += param.n_elem;
    }
    return (1 + num_of_params) * sizeof(float) + sizeof(size_t);
}

Variance_safezone_func::~Variance_safezone_func() {}


/*********************************************
	safezone
*********************************************/

safezone::safezone() : szone(nullptr) {}

safezone::~safezone() {}

// valid safezone
safezone::safezone(ml_safezone_function *sz) : szone(sz) {}

// Movable
safezone::safezone(safezone &&other) {
    swap(other);
}

safezone &safezone::operator=(safezone &&other) {
    swap(other);
    return *this;
}

// Copyable
safezone::safezone(const safezone &other) {
    szone = other.szone;
}

safezone &safezone::operator=(const safezone &other) {
    if (szone != other.szone) {
        szone = other.szone;
    }
    return *this;
}


/*********************************************
	dl_query_state
*********************************************/

query_state::query_state() {}

query_state::query_state(vector <arma::SizeMat> vsz) {
    for (auto sz:vsz)
        GlobalModel.push_back(arma::mat(sz, arma::fill::zeros));
}

void query_state::initializeGlobalModel(vector <arma::SizeMat> vsz) {
    for (auto sz:vsz)
        GlobalModel.push_back(arma::mat(sz, arma::fill::zeros));
}

void query_state::update_estimate(vector <arma::mat> &mdl) {
    for (size_t i = 0; i < mdl.size(); i++)
        GlobalModel.at(i) -= GlobalModel.at(i) - mdl.at(i);
}

void query_state::update_estimate(vector<arma::mat *> &mdl) {
    for (size_t i = 0; i < mdl.size(); i++)
        GlobalModel.at(i) -= GlobalModel.at(i) - *mdl.at(i);
}

void query_state::update_estimate_v2(vector <arma::mat> &mdl) {
    for (size_t i = 0; i < mdl.size(); i++)
        GlobalModel.at(i) += mdl.at(i);
}

void query_state::update_estimate_v2(vector<arma::mat *> &mdl) {
    for (size_t i = 0; i < mdl.size(); i++)
        GlobalModel.at(i) += *mdl.at(i);
}

query_state::~query_state() {}

ml_safezone_function *query_state::safezone(string cfg, string algo) {

    Json::Value root;
    std::ifstream cfgfl(cfg);
    cfgfl >> root;

    string algorithm = root[algo].get("algorithm", "Variance_Monitoring").asString();
    cout << algorithm << endl;
    if (algorithm == "Batch_Learning") {
        auto safe_zone = new Batch_Learning(GlobalModel, root[algo].get("batch_size", 32).asInt64());
        return safe_zone;
    } else if (algorithm == "Variance_Monitoring") {
        auto safe_zone = new Variance_safezone_func(GlobalModel,
                                                    root[algo].get("threshold", 1.).asDouble(),
                                                    root[algo].get("batch_size", 32).asInt64());
        return safe_zone;
    } else {
        return nullptr;
    }
}


/*********************************************
	continuous_query
*********************************************/

continuous_query::continuous_query(string cfg, string nm) {
    cout << "Initializing the query..." << endl;
    Json::Value root;
    std::ifstream cfgfl(cfg);
    cfgfl >> root;

    config.distributed_learning_algorithm = root["gm_network_" + nm].get("distributed_learning_algorithm",
                                                                         "Trash").asString();
    config.network_name = nm;
    config.precision = root[config.distributed_learning_algorithm].get("precision", 0.01).asFloat();
    config.rebalancing = root[config.distributed_learning_algorithm].get("rebalancing", false).asBool();
    config.reb_mult = root[config.distributed_learning_algorithm].get("reb_mult", -1.).asFloat();
    config.beta_mu = root[config.distributed_learning_algorithm].get("beta_mu", 0.5).asFloat();
    config.max_rebs = root[config.distributed_learning_algorithm].get("max_rebs", 2).asInt64();

    config.cfgfile = cfg;

    cout << "Query initialized : ";
    cout << config.distributed_learning_algorithm << ", ";
    cout << config.network_name << ", ";
    cout << config.cfgfile << endl;
}

void continuous_query::setTestSet(arma::mat *tSet, arma::mat *tRes) {
    testSet = tSet;
    testResponses = tRes;
}

double continuous_query::queryAccuracy(RNNPredictor *rnn) { return rnn->getModelAccuracy(); }



/*********************************************
	dl_safezone_function
*********************************************/

//dl_safezone_function::dl_safezone_function(vector<resizable_tensor *> &mdl) : GlobalModel(mdl) {
//    num_of_params = 0;
//    for (auto layer:mdl) {
//        num_of_params += layer->size();
//    }
//}
//
//dl_safezone_function::~dl_safezone_function() {}
//
//const vector<resizable_tensor *> &dl_safezone_function::getGlobalModel() const {
//    return GlobalModel;
//}


/*********************************************
	Batch_safezone_function
*********************************************/

//Batch_safezone_function::Batch_safezone_function(vector<resizable_tensor *> &GlMd) : dl_safezone_function(GlMd), threshold(32.) {
//    hyperparameters.push_back(32.);
//}
//
//Batch_safezone_function::Batch_safezone_function(vector<resizable_tensor *> &GlMd, size_t thr) : dl_safezone_function(GlMd), threshold(thr) {
//    hyperparameters.push_back(thr);
//}
//
//size_t Batch_safezone_function::checkIfAdmissible(const size_t counter) const {
//    return threshold - counter;
//}
//
//Batch_safezone_function::~Batch_safezone_function() {}


/*********************************************
	dl_safezone
*********************************************/

//dl_safezone::dl_safezone() : szone(nullptr) {}
//
//dl_safezone::~dl_safezone() {}
//
//// valid dl_safezone
//dl_safezone::dl_safezone(dl_safezone_function *sz) : szone(sz) {}
//
//// Movable
//dl_safezone::dl_safezone(dl_safezone &&other) {
//    swap(other);
//}
//
//dl_safezone &dl_safezone::operator=(dl_safezone &&other) {
//    swap(other);
//    return *this;
//}
//
//// Copyable
//dl_safezone::dl_safezone(const dl_safezone &other) {
//    szone = other.szone;
//}
//
//dl_safezone &dl_safezone::operator=(const dl_safezone &other) {
//    if (szone != other.szone) {
//        szone = other.szone;
//    }
//    return *this;
//}
//
//
/*********************************************
	dl_query_state
*********************************************/
//
//dl_query_state::dl_query_state() {}
//
//dl_query_state::dl_query_state(vector<resizable_tensor *> &mdl) {
//    num_of_params = 0;
//    for (auto layer:mdl) {
//        resizable_tensor *l;
//        l = new resizable_tensor();
//        l->set_size(layer->num_samples(), layer->k(), layer->nr(), layer->nc());
//        *l = 0.;
//        GlobalModel.push_back(l);
//        num_of_params += layer->size();
//    }
//}
//
//void dl_query_state::initializeGlobalModel(const vector<tensor *> &mdl) {
//    num_of_params = 0;
//    for (auto layer:mdl) {
//        resizable_tensor *l;
//        l = new resizable_tensor();
//        l->set_size(layer->num_samples(), layer->k(), layer->nr(), layer->nc());
//        *l = 0.;
//        GlobalModel.push_back(l);
//        num_of_params += layer->size();
//    }
//}
//
//void dl_query_state::update_estimate(vector<resizable_tensor *> &mdl) {
//    for (size_t i = 0; i < GlobalModel.size(); i++) {
//        //dlib::cpu::affine_transform(*GlobalModel.at(i), *mdl.at(i), 1., 0.);
//        dlib::cpu::affine_transform(*GlobalModel.at(i), *mdl.at(i), 1.);
//    }
//}
//
//dl_query_state::~dl_query_state() {}
//
//dl_safezone_function *dl_query_state::dl_safezone(string cfg, string algo) {
//
//    Json::Value root;
//    std::ifstream cfgfl(cfg);
//    cfgfl >> root;
//
//    string algorithm = root[algo].get("algorithm", "Variance_Monitoring").asString();
//    if (algorithm == "Batch_Learning") {
//        auto safe_zone = new Batch_safezone_function(GlobalModel, root[algo].get("batch_size", 32).asInt64());
//        return safe_zone;
//    } else if (algorithm == "Variance_Monitoring") {
//        auto safe_zone = new Param_Variance_safezone_func(GlobalModel, root[algo].get("threshold", 1.).asDouble(),  root[algo].get("batch_size", 32).asInt64());
//        return safe_zone;
//    } else {
//        return nullptr;
//    }
//}


/*********************************************
	tcp_channel
*********************************************/

tcp_channel::tcp_channel(host *src, host *dst, rpcc_t endp) : channel(src, dst, endp), tcp_byts(0) {}

void tcp_channel::transmit(size_t msg_size) {
    // update parent statistics
    channel::transmit(msg_size);

    // update tcp byte count
    size_t segno = (msg_size + tcp_mss - 1) / tcp_mss;
    tcp_byts += msg_size + segno * tcp_header_bytes;
}
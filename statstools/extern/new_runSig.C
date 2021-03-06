/*
Author: Aaron Armbruster
Date:   2012-06-01
Email:  armbrusa@umich.edu
Description: 

Compute statistical significance with profile likelihood test stat. 
Option for uncapped test stat is added (doUncap), as well as an option
to choose which mu value to profile observed data at before generating expected


Modified by Noel Dawe
*/

#include <iostream>
#include <sstream>
#include <iomanip>

#include "TH1D.h"
#include "TFile.h"
#include "Math/MinimizerOptions.h"

#include "RooCategory.h"
#include "RooWorkspace.h"
#include "RooStats/ModelConfig.h"
#include "RooDataSet.h"
#include "RooMinimizerFcn.h"
#include "RooMinimizer.h"
#include "RooNLLVar.h"
#include "RooRealVar.h"
#include "RooSimultaneous.h"
#include "TSystem.h"


using namespace std;
using namespace RooFit;
using namespace RooStats;


RooDataSet* make_asimov_data(
    RooWorkspace* w, ModelConfig* mc,
    RooNLLVar* conditioning_nll = NULL,
    double mu_val = 1., double mu_val_profile = 1.,
    bool floating_mu_val_profile = false,
    string* mu_str = NULL, string* mu_prof_str = NULL,
    int print_level = 0);

RooDataSet* makeAsimovData(
    RooWorkspace* w, ModelConfig* mc,
    RooNLLVar* conditioning_nll = NULL,
    double mu_val = 1., double mu_val_profile = 1.,
    bool floating_mu_val_profile = false,
    string* mu_str = NULL, string* mu_prof_str = NULL,
    int print_level = 0);

int minimize(RooNLLVar* nll);

double get_sig(RooWorkspace* ws, RooNLLVar* nll, RooRealVar* mu);


TH1D* significance(RooWorkspace* ws,
                   bool observed = false,            // compute observed significance
                   double injection_mu = 1,          // mu injected in the asimov data
                   bool injection_test = false,      // setup the poi for injection study (false is faster if you're not)
                   bool profile = false,             // profile the observed data before generating the Asimov
                   double profile_mu = 1,            // mu value to profile the observed data at before generating the Asimov
                   bool floating_profile_mu = false, // if true then profile at mu hat
                   const char* modelConfigName = "ModelConfig",
                   const char* dataName = "obsData")
{
    string defaultMinimizer = "Minuit2"; // or "Minuit"
    int defaultStrategy     = 1;         // Minimization strategy. 0-2. 0 = fastest, least robust. 2 = slowest, most robust

    if (!ws)
    {
        cout << "ERROR::Workspace is NULL!" << endl;
        return NULL;
    }
    ModelConfig* mc = (ModelConfig*)ws->obj(modelConfigName);
    if (!mc)
    {
        cout << "ERROR::ModelConfig: " << modelConfigName << " doesn't exist!" << endl;
        return NULL;
    }
    RooDataSet* data = (RooDataSet*)ws->data(dataName);
    if (!data)
    {
        cout << "ERROR::Dataset: " << dataName << " doesn't exist!" << endl;
        return NULL;
    }
    
    // save original state
    ws->saveSnapshot("significance::nominal_globs", *mc->GetGlobalObservables());
    ws->saveSnapshot("significance::nominal_nuis", *mc->GetNuisanceParameters());
    ws->saveSnapshot("significance::nominal_poi", *mc->GetParametersOfInterest());

    // minimizer options
    ROOT::Math::MinimizerOptions::SetDefaultMinimizer(defaultMinimizer.c_str());
    ROOT::Math::MinimizerOptions::SetDefaultStrategy(defaultStrategy);
    ROOT::Math::MinimizerOptions::SetDefaultPrintLevel(1);
    //RooNLLVar::SetIgnoreZeroEntries(1);
    //ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(20000);
    //RooMinimizer::SetMaxFunctionCalls(10000);
    
    RooRealVar* mu = (RooRealVar*)mc->GetParametersOfInterest()->first();
    RooArgSet nuis = *mc->GetNuisanceParameters();
    RooArgSet globs = *mc->GetGlobalObservables();
    RooAbsPdf* pdf = mc->GetPdf();
    RooNLLVar* nll = NULL;

    if (observed)
    {
        // observed NLL
        nll = (RooNLLVar*)pdf->createNLL(*data, Constrain(nuis), GlobalObservables(globs));
    }
    else
    {
        RooNLLVar* profile_nll = NULL;
        if (profile)
            profile_nll = (RooNLLVar*)pdf->createNLL(*data, Constrain(nuis), GlobalObservables(globs));
        // make asimov data
        string mu_str, mu_prof_str;
        /*
        RooDataSet* asimov_data = make_asimov_data(
            ws, mc, profile_nll,
            injection_mu, profile_mu,
            floating_profile_mu,
            &mu_str, &mu_prof_str);
        */
        RooDataSet* asimov_data = makeAsimovData(
            ws, mc, profile_nll,
            injection_mu, profile_mu,
            floating_profile_mu,
            &mu_str, &mu_prof_str);
        // asimov NLL
        nll = (RooNLLVar*)pdf->createNLL(*asimov_data, Constrain(nuis), GlobalObservables(globs));
    }
    
    // compute the significance
    double sig = get_sig(ws, nll, mu);
    if (sig == -999)
        return NULL;
            
    TH1D* h_hypo = new TH1D("significance", "significance", 2, 0, 2);
    h_hypo->SetBinContent(1, sig);
    h_hypo->SetBinContent(2, mu->getVal());
    h_hypo->SetBinError(2, mu->getError());
    
    // restore original state
    ws->loadSnapshot("significance::nominal_globs");
    ws->loadSnapshot("significance::nominal_nuis");
    ws->loadSnapshot("significance::nominal_poi");

    return h_hypo;
}


double get_sig(RooWorkspace* ws, RooNLLVar* nll, RooRealVar* mu)
{
    int status, sign;
    double sig=0, q0=0;

    // restore nominal state
    ws->loadSnapshot("significance::nominal_globs");
    ws->loadSnapshot("significance::nominal_nuis");

    // conditional fit with mu=0
    mu->setVal(0);
    mu->setConstant(1);
    status = minimize(nll);
    if (status < 0) 
    {
        cout << "ERROR: FIT FAILED" << endl;
        return -999.;
    }
    double nll_cond = nll->getVal();

    // restore nominal state
    ws->loadSnapshot("significance::nominal_globs");
    ws->loadSnapshot("significance::nominal_nuis");
    
    // unconditional fit
    mu->setVal(0);
    mu->setConstant(0);
    status = minimize(nll);
    if (status < 0) 
    {
        cout << "ERROR: FIT FAILED" << endl;
        return -999.;
    }
    double nll_min = nll->getVal();

    q0 = 2*(nll_cond - nll_min);

    sign = int(q0 == 0 ? 0 : q0 / fabs(q0));
    sig = sign * sqrt(fabs(q0));

    cout << "test stat val: " << q0 << endl;
    cout << "significance:  " << sig << endl;
    return sig;
}


int minimize(RooNLLVar* nll)
{
    int printLevel = ROOT::Math::MinimizerOptions::DefaultPrintLevel();
    RooFit::MsgLevel msglevel = RooMsgService::instance().globalKillBelow();
    if (printLevel < 0)
        RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);
    int strat = ROOT::Math::MinimizerOptions::DefaultStrategy();
    RooMinimizer minim(*nll);
    minim.setStrategy(strat);
    minim.setPrintLevel(printLevel);

    int status = minim.minimize(ROOT::Math::MinimizerOptions::DefaultMinimizerType().c_str(), ROOT::Math::MinimizerOptions::DefaultMinimizerAlgo().c_str());

    if (status != 0 && status != 1 && strat < 2)
    {
        strat++;
        cout << "Fit failed with status " << status << ". Retrying with strategy " << strat << endl;
        minim.setStrategy(strat);
        status = minim.minimize(ROOT::Math::MinimizerOptions::DefaultMinimizerType().c_str(), ROOT::Math::MinimizerOptions::DefaultMinimizerAlgo().c_str());
    }

    if (status != 0 && status != 1 && strat < 2)
    {
        strat++;
        cout << "Fit failed with status " << status << ". Retrying with strategy " << strat << endl;
        minim.setStrategy(strat);
        status = minim.minimize(ROOT::Math::MinimizerOptions::DefaultMinimizerType().c_str(), ROOT::Math::MinimizerOptions::DefaultMinimizerAlgo().c_str());
    }

    if (status != 0 && status != 1)
    {
        cout << "Fit failed with status " << status << endl;
        string minType = ROOT::Math::MinimizerOptions::DefaultMinimizerType();
        string newMinType;
        if (minType == "Minuit2") newMinType = "Minuit";
        else newMinType = "Minuit2";

        cout << "Switching minuit type from " << minType << " to " << newMinType << endl;

        ROOT::Math::MinimizerOptions::SetDefaultMinimizer(newMinType.c_str());
        strat = 1; //ROOT::Math::MinimizerOptions::DefaultStrategy();
        minim.setStrategy(strat);

        status = minim.minimize(ROOT::Math::MinimizerOptions::DefaultMinimizerType().c_str(), ROOT::Math::MinimizerOptions::DefaultMinimizerAlgo().c_str());


        if (status != 0 && status != 1 && strat < 2)
        {
            strat++;
            cout << "Fit failed with status " << status << ". Retrying with strategy " << strat << endl;
            minim.setStrategy(strat);
            status = minim.minimize(ROOT::Math::MinimizerOptions::DefaultMinimizerType().c_str(), ROOT::Math::MinimizerOptions::DefaultMinimizerAlgo().c_str());
        }

        if (status != 0 && status != 1 && strat < 2)
        {
            strat++;
            cout << "Fit failed with status " << status << ". Retrying with strategy " << strat << endl;
            minim.setStrategy(strat);
            status = minim.minimize(ROOT::Math::MinimizerOptions::DefaultMinimizerType().c_str(), ROOT::Math::MinimizerOptions::DefaultMinimizerAlgo().c_str());
        }
        strat=2;
        ROOT::Math::MinimizerOptions::SetDefaultMinimizer(minType.c_str());
    }

    if (printLevel < 0)
        RooMsgService::instance().setGlobalKillBelow(msglevel);

    return status;
}


void unfold_constraints(RooArgSet& initial, RooArgSet& final, RooArgSet& obs, RooArgSet& nuis, int& counter)
{
    if (counter > 50)
    {
        cout << "ERROR::Couldn't unfold constraints!" << endl;
        cout << "Initial: " << endl;
        initial.Print("v");
        cout << endl;
        cout << "Final: " << endl;
        final.Print("v");
        exit(1);
    }
    TIterator* itr = initial.createIterator();
    RooAbsPdf* pdf;
    while ((pdf = (RooAbsPdf*)itr->Next()))
    {
        RooArgSet nuis_tmp = nuis;
        RooArgSet constraint_set(*pdf->getAllConstraints(obs, nuis_tmp, false));
        string className(pdf->ClassName());
        if (className != "RooGaussian" && className != "RooLognormal" && className != "RooGamma" && className != "RooPoisson" && className != "RooBifurGauss")
        {
            counter++;
            unfold_constraints(constraint_set, final, obs, nuis, counter);
        }
        else
        {
            final.add(*pdf);
        }
    }
    delete itr;
}


RooDataSet* makeAsimovData(RooWorkspace* w, ModelConfig* mc,
                           RooNLLVar* conditioning_nll,
                           double mu_val, double mu_val_profile,
                           bool floating_mu_val_profile,
                           string* mu_str, string* mu_prof_str,
                           int print_level)
{

    if (mu_val_profile == -999) mu_val_profile = mu_val;


    cout << "Creating asimov data at mu = " << mu_val << ", profiling at mu = " << mu_val_profile << endl;

    //ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2");
    //int strat = ROOT::Math::MinimizerOptions::SetDefaultStrategy(0);
    //int printLevel = ROOT::Math::MinimizerOptions::DefaultPrintLevel();
    //ROOT::Math::MinimizerOptions::SetDefaultPrintLevel(-1);
    //RooMinuit::SetMaxIterations(10000);
    //RooMinimizer::SetMaxFunctionCalls(10000);

    ////////////////////
    //make asimov data//
    ////////////////////

    RooAbsPdf* combPdf = mc->GetPdf();

    int _printLevel = 0;

    stringstream muStr;
    muStr << setprecision(5);
    muStr << "_" << mu_val;
    if (mu_str) *mu_str = muStr.str();

    stringstream muStrProf;
    muStrProf << setprecision(5);
    muStrProf << "_" << mu_val_profile;
    if (mu_prof_str) *mu_prof_str = muStrProf.str();

    RooRealVar* mu = (RooRealVar*)mc->GetParametersOfInterest()->first();//w->var("mu");
    mu->setVal(mu_val);

    RooArgSet mc_obs = *mc->GetObservables();
    RooArgSet mc_globs = *mc->GetGlobalObservables();
    RooArgSet mc_nuis = *mc->GetNuisanceParameters();

    //pair the nuisance parameter to the global observable
    RooArgSet mc_nuis_tmp = mc_nuis;
    RooArgList nui_list("ordered_nuis");
    RooArgList glob_list("ordered_globs");
    RooArgSet constraint_set_tmp(*combPdf->getAllConstraints(mc_obs, mc_nuis_tmp, false));
    RooArgSet constraint_set;
    int counter_tmp = 0;
    unfold_constraints(constraint_set_tmp, constraint_set, mc_obs, mc_nuis_tmp, counter_tmp);

    TIterator* cIter = constraint_set.createIterator();
    RooAbsArg* arg;

    /// Go through all constraints
    while ((arg = (RooAbsArg*)cIter->Next()))
    {
        RooAbsPdf* pdf = (RooAbsPdf*)arg;
        if (!pdf) continue;
//             cout << "Printing pdf" << endl;
//             pdf->Print();
//             cout << "Done" << endl;

        /// Catch the nuisance parameter constrained here
        TIterator* nIter = mc_nuis.createIterator();
        RooRealVar* thisNui = NULL;
        RooAbsArg* nui_arg;
        while ((nui_arg = (RooAbsArg*)nIter->Next()))
        {
            if (pdf->dependsOn(*nui_arg))
            {
                thisNui = (RooRealVar*)nui_arg;
                break;
            }
        }
        delete nIter;

        //RooRealVar* thisNui = (RooRealVar*)pdf->getObservables();


        //need this incase the observable isn't fundamental. 
        //in this case, see which variable is dependent on the nuisance parameter and use that.
        RooArgSet* components = pdf->getComponents();
//             cout << "\nPrinting components" << endl;
//             components->Print();
//             cout << "Done" << endl;
        components->remove(*pdf);
        if (components->getSize())
        {
            TIterator* itr1 = components->createIterator();
            RooAbsArg* arg1;
            while ((arg1 = (RooAbsArg*)itr1->Next()))
            {
                TIterator* itr2 = components->createIterator();
                RooAbsArg* arg2;
                while ((arg2 = (RooAbsArg*)itr2->Next()))
                {
                    if (arg1 == arg2) continue;
                    if (arg2->dependsOn(*arg1))
                    {
                        components->remove(*arg1);
                    }
                }
                delete itr2;
            }
            delete itr1;
        }
        if (components->getSize() > 1)
        {
            cout << "ERROR::Couldn't isolate proper nuisance parameter" << endl;
            return NULL;
        }
        else if (components->getSize() == 1)
        {
            thisNui = (RooRealVar*)components->first();
        }

        TIterator* gIter = mc_globs.createIterator();
        RooRealVar* thisGlob = NULL;
        RooAbsArg* glob_arg;
        while ((glob_arg = (RooAbsArg*)gIter->Next()))
        {
            if (pdf->dependsOn(*glob_arg))
            {
                thisGlob = (RooRealVar*)glob_arg;
                break;
            }
        }
        delete gIter;

        if (!thisNui || !thisGlob)
        {
            cout << "WARNING::Couldn't find nui or glob for constraint: " << pdf->GetName() << endl;
            //return;
            continue;
        }

        if (_printLevel >= 1) cout << "Pairing nui: " << thisNui->GetName() << ", with glob: " << thisGlob->GetName() << ", from constraint: " << pdf->GetName() << endl;

        nui_list.add(*thisNui);
        glob_list.add(*thisGlob);

        //     cout << "\nPrinting Nui/glob" << endl;
        //     thisNui->Print();
        //     cout << "Done nui" << endl;
        //     thisGlob->Print();
        //     cout << "Done glob" << endl;
    }
    delete cIter;
    
    /*  
    //save the snapshots of nominal parameters, but only if they're not already saved
    w->saveSnapshot("tmpGlobs",*mc->GetGlobalObservables());
    w->saveSnapshot("tmpNuis",*mc->GetNuisanceParameters());
    if (!w->loadSnapshot("nominalGlobs"))
    {
        //cout << "nominalGlobs doesn't exist. Saving snapshot." << endl;
        w->saveSnapshot("nominalGlobs",*mc->GetGlobalObservables());
    }
    else w->loadSnapshot("tmpGlobs");
    if (!w->loadSnapshot("nominalNuis"))
    {
        //cout << "nominalNuis doesn't exist. Saving snapshot." << endl;
        w->saveSnapshot("nominalNuis",*mc->GetNuisanceParameters());
    }
    else w->loadSnapshot("tmpNuis");
    */
    RooArgSet nuiSet_tmp(nui_list);
    
    mu->setVal(mu_val_profile);
    mu->setConstant(1);
    //int status = 0;

    if (conditioning_nll != NULL)
    {
        minimize(conditioning_nll);

        // cout << "Using globs for minimization" << endl;
        // mc->GetGlobalObservables()->Print("v");
        // cout << "Starting minimization.." << endl;
        // RooAbsReal* nll;
        // if (!(nll = map_data_nll[combData])) nll = combPdf->createNLL(*combData, RooFit::Constrain(nuiSet_tmp));
        // RooMinimizer minim(*nll);
        // minim.setStrategy(0);
        // minim.setPrintLevel(1);
        // status = minim.minimize(ROOT::Math::MinimizerOptions::DefaultMinimizerType().c_str(), ROOT::Math::MinimizerOptions::DefaultMinimizerAlgo().c_str());
        // if (status != 0)
        // {
        //   cout << "Fit failed for mu = " << mu->getVal() << " with status " << status << endl;
        // }
        // cout << "Done" << endl;

        //combPdf->fitTo(*combData,Hesse(false),Minos(false),PrintLevel(0),Extended(), Constrain(nuiSet_tmp));
    }
    mu->setConstant(0);
    mu->setVal(mu_val);

    //loop over the nui/glob list, grab the corresponding variable from the tmp ws, and set the glob to the value of the nui
    int nrNuis = nui_list.getSize();
    if (nrNuis != glob_list.getSize())
    {
        cout << "ERROR::nui_list.getSize() != glob_list.getSize()!" << endl;
        return NULL;
    }

    for (int i=0;i<nrNuis;i++)
    {
        RooRealVar* nui = (RooRealVar*)nui_list.at(i);
        RooRealVar* glob = (RooRealVar*)glob_list.at(i);

        //cout << "nui: " << nui << ", glob: " << glob << endl;
        //cout << "Setting glob: " << glob->GetName() << ", which had previous val: " << glob->getVal() << ", to conditional val: " << nui->getVal() << endl;

        glob->setVal(nui->getVal());
    }

    //save the snapshots of conditional parameters
    //cout << "Saving conditional snapshots" << endl;
    //cout << "Glob snapshot name = " << "conditionalGlobs"+muStrProf.str() << endl;
    //cout << "Nuis snapshot name = " << "conditionalNuis"+muStrProf.str() << endl;
    w->saveSnapshot(("conditionalGlobs"+muStrProf.str()).c_str(),*mc->GetGlobalObservables());
    w->saveSnapshot(("conditionalNuis" +muStrProf.str()).c_str(),*mc->GetNuisanceParameters());

    if (conditioning_nll == NULL)
    {
        w->loadSnapshot("nominalGlobs");
        w->loadSnapshot("nominalNuis");
    }

    if (_printLevel >= 1) cout << "Making asimov" << endl;
    //make the asimov data (snipped from Kyle)
    mu->setVal(mu_val);

    int iFrame=0;

    const char* weightName="weightVar";
    RooArgSet obsAndWeight;
    //cout << "adding obs" << endl;
    obsAndWeight.add(*mc->GetObservables());
    //cout << "adding weight" << endl;

    RooRealVar* weightVar = NULL;
    if (!(weightVar = w->var(weightName)))
    {
        w->import(*(new RooRealVar(weightName, weightName, 1,0,10000000)));
        weightVar = w->var(weightName);
    }
    //cout << "weightVar: " << weightVar << endl;
    obsAndWeight.add(*w->var(weightName));

    //cout << "defining set" << endl;
    w->defineSet("obsAndWeight",obsAndWeight);

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    // MAKE ASIMOV DATA FOR OBSERVABLES

    // dummy var can just have one bin since it's a dummy
    //if(w->var("ATLAS_dummyX"))  w->var("ATLAS_dummyX")->setBins(1);

    //cout <<" check expectedData by category"<<endl;
    //RooDataSet* simData=NULL;
    RooSimultaneous* simPdf = dynamic_cast<RooSimultaneous*>(mc->GetPdf());

    RooDataSet* asimovData;
    if (!simPdf)
    {
        // Get pdf associated with state from simpdf
        RooAbsPdf* pdftmp = mc->GetPdf();//simPdf->getPdf(channelCat->getLabel()) ;

        // Generate observables defined by the pdf associated with this state
        RooArgSet* obstmp = pdftmp->getObservables(*mc->GetObservables()) ;

        if (_printLevel >= 1)
        {
            obstmp->Print();
        }

        asimovData = new RooDataSet(("asimovData"+muStr.str()).c_str(),("asimovData"+muStr.str()).c_str(),RooArgSet(obsAndWeight),WeightVar(*weightVar));

        RooRealVar* thisObs = ((RooRealVar*)obstmp->first());
        double expectedEvents = pdftmp->expectedEvents(*obstmp);
        double thisNorm = 0;
        for(int jj=0; jj<thisObs->numBins(); ++jj){
            thisObs->setBin(jj);

            thisNorm=pdftmp->getVal(obstmp)*thisObs->getBinWidth(jj);
            if (thisNorm*expectedEvents <= 0)
            {
                cout << "WARNING::Detected bin with zero expected events (" << thisNorm*expectedEvents << ") ! Please check your inputs. Obs = " << thisObs->GetName() << ", bin = " << jj << endl;
            }
            if (thisNorm*expectedEvents > 0 && thisNorm*expectedEvents < pow(10.0, 18)) asimovData->add(*mc->GetObservables(), thisNorm*expectedEvents);
        }

        if (_printLevel >= 1)
        {
            asimovData->Print();
            cout <<"sum entries "<<asimovData->sumEntries()<<endl;
        }
        if(asimovData->sumEntries()!=asimovData->sumEntries()){
            cout << "sum entries is nan"<<endl;
            exit(1);
        }

        //((RooRealVar*)obstmp->first())->Print();
        //cout << "expected events " << pdftmp->expectedEvents(*obstmp) << endl;

        w->import(*asimovData);

        if (_printLevel >= 1)
        {
            asimovData->Print();
            cout << endl;
        }
    }
    else
    {
        map<string, RooDataSet*> asimovDataMap;

        //try fix for sim pdf
        RooCategory* channelCat = (RooCategory*)&simPdf->indexCat();//(RooCategory*)w->cat("master_channel");//(RooCategory*) (&simPdf->indexCat());
        //    TIterator* iter = simPdf->indexCat().typeIterator() ;
        TIterator* iter = channelCat->typeIterator() ;
        RooCatType* tt = NULL;
        int nrIndices = 0;
        while((tt=(RooCatType*) iter->Next())) {
            nrIndices++;
        }
        for (int i=0;i<nrIndices;i++){
            channelCat->setIndex(i);
            iFrame++;
            // Get pdf associated with state from simpdf
            RooAbsPdf* pdftmp = simPdf->getPdf(channelCat->getLabel()) ;

            // Generate observables defined by the pdf associated with this state
            RooArgSet* obstmp = pdftmp->getObservables(*mc->GetObservables()) ;

            if (_printLevel >= 1)
            {
                obstmp->Print();
                cout << "on type " << channelCat->getLabel() << " " << iFrame << endl;
            }

            RooDataSet* obsDataUnbinned = new RooDataSet(Form("combAsimovData%d",iFrame),Form("combAsimovData%d",iFrame),RooArgSet(obsAndWeight,*channelCat),WeightVar(*weightVar));
            RooRealVar* thisObs = ((RooRealVar*)obstmp->first());
            double expectedEvents = pdftmp->expectedEvents(*obstmp);
            double thisNorm = 0;
            for(int jj=0; jj<thisObs->numBins(); ++jj){
                thisObs->setBin(jj);

                thisNorm=pdftmp->getVal(obstmp)*thisObs->getBinWidth(jj);
                if (thisNorm*expectedEvents > 0 && thisNorm*expectedEvents < pow(10.0, 18)) obsDataUnbinned->add(*mc->GetObservables(), thisNorm*expectedEvents);
            }

            if (_printLevel >= 1)
            {
                obsDataUnbinned->Print();
                cout <<"sum entries "<<obsDataUnbinned->sumEntries()<<endl;
            }
            if(obsDataUnbinned->sumEntries()!=obsDataUnbinned->sumEntries()){
                cout << "sum entries is nan"<<endl;
                exit(1);
            }

            // ((RooRealVar*)obstmp->first())->Print();
            // cout << "pdf: " << pdftmp->GetName() << endl;
            // cout << "expected events " << pdftmp->expectedEvents(*obstmp) << endl;
            // cout << "-----" << endl;

            asimovDataMap[string(channelCat->getLabel())] = obsDataUnbinned;//tempData;

            if (_printLevel >= 1)
            {
                cout << "channel: " << channelCat->getLabel() << ", data: ";
                obsDataUnbinned->Print();
                cout << endl;
            }
        }

        asimovData = new RooDataSet(("asimovData"+muStr.str()).c_str(),("asimovData"+muStr.str()).c_str(),RooArgSet(obsAndWeight,*channelCat),Index(*channelCat),Import(asimovDataMap),WeightVar(*weightVar));
        w->import(*asimovData);
    }

    //bring us back to nominal for exporting
    //w->loadSnapshot("nominalNuis");
    w->loadSnapshot("nominalGlobs");

    //ROOT::Math::MinimizerOptions::SetDefaultPrintLevel(printLevel);

    return asimovData;
}


RooDataSet* make_asimov_data(RooWorkspace* w, ModelConfig* mc,
                             RooNLLVar* conditioning_nll, 
                             double mu_val, double mu_val_profile,
                             bool floating_mu_val_profile,
                             string* mu_str, string* mu_prof_str,
                             int print_level)
{
    ////////////////////
    //make asimov data//
    ////////////////////

    //ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2");
    //int strat = ROOT::Math::MinimizerOptions::SetDefaultStrategy(0);
    //int printLevel = ROOT::Math::MinimizerOptions::DefaultPrintLevel();
    //ROOT::Math::MinimizerOptions::SetDefaultPrintLevel(-1);
    //RooMinuit::SetMaxIterations(10000);
    //RooMinimizer::SetMaxFunctionCalls(10000);
    
    RooAbsPdf* combPdf = mc->GetPdf();

    stringstream muStr;
    muStr << setprecision(5);
    muStr << "_" << mu_val;
    if (mu_str) *mu_str = muStr.str();

    stringstream muStrProf;
    muStrProf << setprecision(5);
    muStrProf << "_" << mu_val_profile;
    if (mu_prof_str) *mu_prof_str = muStrProf.str();

    RooRealVar* mu = (RooRealVar*)mc->GetParametersOfInterest()->first();
    mu->setVal(mu_val);

    RooArgSet mc_obs = *mc->GetObservables();
    RooArgSet mc_globs = *mc->GetGlobalObservables();
    RooArgSet mc_nuis = *mc->GetNuisanceParameters();

    // pair the nuisance parameter to the global observable
    RooArgSet mc_nuis_tmp = mc_nuis;
    RooArgList nui_list("ordered_nuis");
    RooArgList glob_list("ordered_globs");
    RooArgSet constraint_set_tmp(*combPdf->getAllConstraints(mc_obs, mc_nuis_tmp, false));
    RooArgSet constraint_set;
    int counter_tmp = 0;
    unfold_constraints(constraint_set_tmp, constraint_set, mc_obs, mc_nuis_tmp, counter_tmp);

    TIterator* cIter = constraint_set.createIterator();
    RooAbsArg* arg;

    while ((arg = (RooAbsArg*)cIter->Next()))
    {
        RooAbsPdf* pdf = (RooAbsPdf*)arg;
        if (!pdf)
            continue;

        /// Catch the nuisance parameter constrained here
        TIterator* nIter = mc_nuis.createIterator();
        RooRealVar* thisNui = NULL;
        RooAbsArg* nui_arg;
        while ((nui_arg = (RooAbsArg*)nIter->Next()))
        {
            if (pdf->dependsOn(*nui_arg))
            {
                thisNui = (RooRealVar*)nui_arg;
                break;
            }
        }
        delete nIter;

        // need this if the observable isn't fundamental. 
        // in this case, see which variable is dependent on the nuisance parameter and use that.
        RooArgSet* components = pdf->getComponents();
        components->remove(*pdf);

        if (components->getSize())
        {
            TIterator* itr1 = components->createIterator();
            RooAbsArg* arg1;
            while ((arg1 = (RooAbsArg*)itr1->Next()))
            {
                TIterator* itr2 = components->createIterator();
                RooAbsArg* arg2;
                while ((arg2 = (RooAbsArg*)itr2->Next()))
                {
                    if (arg1 == arg2)
                        continue;
                    if (arg2->dependsOn(*arg1))
                    {
                        components->remove(*arg1);
                    }
                }
                delete itr2;
            }
            delete itr1;
        }
        if (components->getSize() > 1)
        {
            cout << "ERROR::Couldn't isolate proper nuisance parameter" << endl;
            return NULL;
        }
        else if (components->getSize() == 1)
        {
            thisNui = (RooRealVar*)components->first();
        }

        TIterator* gIter = mc_globs.createIterator();
        RooRealVar* thisGlob = NULL;
        RooAbsArg* glob_arg;
        while ((glob_arg = (RooAbsArg*)gIter->Next()))
        {
            if (pdf->dependsOn(*glob_arg))
            {
                thisGlob = (RooRealVar*)glob_arg;
                break;
            }
        }
        delete gIter;

        if (!thisNui || !thisGlob)
        {
            cout << "WARNING::Couldn't find nui or glob for constraint: " << pdf->GetName() << endl;
            continue;
        }

        if (print_level > 0)
        {
            cout << "Pairing nui: " << thisNui->GetName()
                 << ", with glob: " << thisGlob->GetName()
                 << ", from constraint: " << pdf->GetName() << endl;
        }

        nui_list.add(*thisNui);
        glob_list.add(*thisGlob);
    }
    delete cIter;
    
    // save original state
    w->saveSnapshot("make_asimov_data::nominal_globs", *mc->GetGlobalObservables());
    w->saveSnapshot("make_asimov_data::nominal_nuis", *mc->GetNuisanceParameters());

    // conditional profiling
    if (conditioning_nll != NULL)
    {
        if (floating_mu_val_profile)
        {
            // profile at mu hat
            mu->setVal(0);
            mu->setConstant(0);
        }
        else
        {
            mu->setVal(mu_val_profile);
            mu->setConstant(1);
        }
        minimize(conditioning_nll);
    }
    mu->setConstant(0);

    // loop over the nui/glob list, grab the corresponding variable from the
    // tmp ws, and set the glob to the value of the nui
    int nrNuis = nui_list.getSize();
    if (nrNuis != glob_list.getSize())
    {
        cout << "ERROR::nui_list.getSize() != glob_list.getSize()!" << endl;
        return NULL;
    }

    for (int i = 0; i < nrNuis; ++i)
    {
        RooRealVar* nui = (RooRealVar*)nui_list.at(i);
        RooRealVar* glob = (RooRealVar*)glob_list.at(i);
        if (print_level > 0)
        {
            cout << "Setting glob: " << glob->GetName()
                 << ", which had previous val: " << glob->getVal()
                 << ", to conditional val: " << nui->getVal() << endl;
        }
        glob->setVal(nui->getVal());
    }

    // save the snapshots of conditional parameters
    w->saveSnapshot(("make_asimov_data::conditional_globs" + muStr.str()).c_str(), *mc->GetGlobalObservables());
    w->saveSnapshot(("make_asimov_data::conditional_nuis" + muStr.str()).c_str(), *mc->GetNuisanceParameters());
    
    if (conditioning_nll == NULL)
    {
        // restore nominal state
        w->loadSnapshot("make_asimov_data::nominal_globs");
        w->loadSnapshot("make_asimov_data::nominal_nuis");
    }

    // make the asimov data
    mu->setVal(mu_val);

    int iFrame = 0;

    const char* weightName="weightVar";
    RooArgSet obsAndWeight;
    obsAndWeight.add(*mc->GetObservables());

    RooRealVar* weightVar = NULL;
    if (!(weightVar = w->var(weightName)))
    {
        w->import(*(new RooRealVar(weightName, weightName, 1,0,10000000)));
        weightVar = w->var(weightName);
    }
    obsAndWeight.add(*w->var(weightName));
    w->defineSet("obsAndWeight",obsAndWeight);

    //////////////////////////////////////////////////////
    // MAKE ASIMOV DATA FOR OBSERVABLES
    //////////////////////////////////////////////////////

    RooSimultaneous* simPdf = dynamic_cast<RooSimultaneous*>(mc->GetPdf());
    RooDataSet* asimovData;

    if (!simPdf)
    {
        // Get pdf associated with state from simpdf
        RooAbsPdf* pdftmp = mc->GetPdf();//simPdf->getPdf(channelCat->getLabel()) ;

        // Generate observables defined by the pdf associated with this state
        RooArgSet* obstmp = pdftmp->getObservables(*mc->GetObservables()) ;

        if (print_level > 0)
        {
            obstmp->Print();
        }

        asimovData = new RooDataSet(
            ("asimovData"+muStr.str()).c_str(),
            ("asimovData"+muStr.str()).c_str(),
            RooArgSet(obsAndWeight),
            WeightVar(*weightVar));

        RooRealVar* thisObs = ((RooRealVar*)obstmp->first());
        double expectedEvents = pdftmp->expectedEvents(*obstmp);
        double thisNorm = 0;
        for(int jj=0; jj<thisObs->numBins(); ++jj){
            thisObs->setBin(jj);

            thisNorm=pdftmp->getVal(obstmp)*thisObs->getBinWidth(jj);
            if (thisNorm*expectedEvents <= 0)
            {
                cout << "WARNING::Detected bin with zero expected events (" << thisNorm*expectedEvents 
                     << ") ! Please check your inputs. Obs = " << thisObs->GetName()
                     << ", bin = " << jj << endl;
            }
            if (thisNorm*expectedEvents > 0 && thisNorm*expectedEvents < pow(10.0, 18))
            {
                asimovData->add(*mc->GetObservables(), thisNorm*expectedEvents);
            }
        }

        if (print_level > 0)
        {
            asimovData->Print();
            cout <<"sum entries "<<asimovData->sumEntries()<<endl;
        }
        if(asimovData->sumEntries()!=asimovData->sumEntries()){
            cout << "sum entries is nan"<<endl;
            return NULL;
        }
        if (print_level > 0)
        {
            asimovData->Print();
            cout << endl;
        }
    }
    else
    {
        map<string, RooDataSet*> asimovDataMap;
        RooCategory* channelCat = (RooCategory*)&simPdf->indexCat();
        TIterator* iter = channelCat->typeIterator() ;
        RooCatType* tt = NULL;
        int nrIndices = 0;
        while((tt=(RooCatType*) iter->Next())) {
            nrIndices++;
        }
        for (int i=0; i < nrIndices; ++i){
            channelCat->setIndex(i);
            ++iFrame;
            // Get pdf associated with state from simpdf
            RooAbsPdf* pdftmp = simPdf->getPdf(channelCat->getLabel()) ;
            // Generate observables defined by the pdf associated with this state
            RooArgSet* obstmp = pdftmp->getObservables(*mc->GetObservables()) ;

            if (print_level > 0)
            {
                obstmp->Print();
                cout << "on type " << channelCat->getLabel() << " " << iFrame << endl;
            }

            RooDataSet* obsDataUnbinned = new RooDataSet(
                Form("combAsimovData%d",iFrame),
                Form("combAsimovData%d",iFrame),
                RooArgSet(obsAndWeight,*channelCat),
                WeightVar(*weightVar));

            RooRealVar* thisObs = ((RooRealVar*)obstmp->first());
            double expectedEvents = pdftmp->expectedEvents(*obstmp);
            double thisNorm = 0;
            for(int jj=0; jj<thisObs->numBins(); ++jj){
                thisObs->setBin(jj);
                thisNorm = pdftmp->getVal(obstmp) * thisObs->getBinWidth(jj);
                if (thisNorm*expectedEvents > 0 && thisNorm*expectedEvents < pow(10.0, 18))
                {
                    obsDataUnbinned->add(*mc->GetObservables(), thisNorm*expectedEvents);
                }
            }

            if (print_level > 0)
            {
                obsDataUnbinned->Print();
                cout <<"sum entries "<<obsDataUnbinned->sumEntries()<<endl;
            }
            if(obsDataUnbinned->sumEntries()!=obsDataUnbinned->sumEntries()){
                cout << "sum entries is nan"<<endl;
                return NULL;
            }

            asimovDataMap[string(channelCat->getLabel())] = obsDataUnbinned;

            if (print_level > 0)
            {
                cout << "channel: " << channelCat->getLabel() << ", data: ";
                obsDataUnbinned->Print();
                cout << endl;
            }
        }

        asimovData = new RooDataSet(
            ("asimovData" + muStr.str()).c_str(),
            ("asimovData" + muStr.str()).c_str(),
            RooArgSet(obsAndWeight,*channelCat),
            Index(*channelCat),
            Import(asimovDataMap),
            WeightVar(*weightVar));
    }

    // restore original state
    w->loadSnapshot("make_asimov_data::nominal_globs");
    w->loadSnapshot("make_asimov_data::nominal_nuis");

    return asimovData;
}

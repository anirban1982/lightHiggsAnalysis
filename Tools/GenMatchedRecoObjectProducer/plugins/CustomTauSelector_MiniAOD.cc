// -*- C++ -*-
//
// Package:    GenMatchedRecoObjectProducer
// Class:      CustomTauSelector_MiniAOD
// 
/**\class CustomTauSelector_MiniAOD CustomTauSelector_MiniAOD.cc 
   BoostedTauAnalysis/GenMatchedRecoObjectProducer/src/CustomTauSelector_MiniAOD.cc

 Description: create a collection of custom selected hadronic taus to put in the event, and stop 
 processing if no taus are selected

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Rachel Yohay,512 1-010,+41227670495,
//         Created:  Fri Aug 24 17:10:12 CEST 2012
// $Id: CustomTauSelector_MiniAOD.cc,v 1.6 2012/12/12 16:02:05 yohay Exp $
//
//


// system include files
#include <memory>
#include <string>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDFilter.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "Tools/Common/interface/Common.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/PhotonFwd.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TFile.h"

#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/PATObject.h"
#include "DataFormats/Candidate/interface/CompositePtrCandidate.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
//
// class declaration
//
using namespace edm;
template<class T>
class CustomTauSelector_MiniAOD : public edm::EDFilter {
public:
  explicit CustomTauSelector_MiniAOD(const edm::ParameterSet&);
  ~CustomTauSelector_MiniAOD();
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  virtual void beginJob();
  virtual bool filter(edm::Event&, const edm::EventSetup&);
  virtual void endJob();
  virtual bool beginRun(edm::Run&, edm::EventSetup const&);
  virtual bool endRun(edm::Run&, edm::EventSetup const&);
  virtual bool beginLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&);
  virtual bool endLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&);

  // ----------member data ---------------------------

  //input tag for reco tau collection
  edm::EDGetTokenT<pat::TauCollection> tauTag_;

  //input tag for base tau collection
  edm::EDGetTokenT<pat::TauCollection> baseTauTag_;

  //input tag for tau isolation energy
  std::string tauHadIsoTag_;

  //input tag for overlap candidate collection
  edm::EDGetTokenT<edm::RefVector<std::vector<T> > > overlapCandTag_;

  edm::EDGetTokenT<edm::RefVector<std::vector<T> > > overlapCandTag1_;

  //input for checking for nearby muon
  edm::EDGetTokenT<std::vector<pat::Muon> > muons_;
  
//vector of input tags, 1 for each discriminator the tau should pass
  std::vector<std::string> tauDiscriminatorTags_; 

  //flag indicating whether the selected taus should pass or fail the discriminator
  bool passDiscriminator_;

  //pT cut
  double pTMin_;

  //|eta| cut
  double etaMax_;

  //maximum isolation cut
  double isoMax_;

  //overlap candidate matching cut
  double dR_;

  //minimum number of objects that must be found to pass the filter
  unsigned int minNumObjsToPassFilter_;
  
  // dR cone to look if muon was removed or nearby, indicating a di-tau object
  double diTaudR_;

  //  std::map<std::string, TH1D*> histos1D_;
  TFile *out_;
  TH1F *TauPt_;
  TH1F *NPassing_;
  TH1F *TauHadTauMudR_;
  std::string  outFileName_;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
template<class T>
CustomTauSelector_MiniAOD<T>::CustomTauSelector_MiniAOD(const edm::ParameterSet& iConfig) :
  tauTag_(iConfig.existsAs<edm::InputTag>("tauTag") ? 
	  consumes<pat::TauCollection>(iConfig.getParameter<edm::InputTag>("tauTag")) : edm::EDGetTokenT<pat::TauCollection>()),
  baseTauTag_(consumes<pat::TauCollection>(iConfig.getParameter<edm::InputTag>("baseTauTag"))),
  tauHadIsoTag_(iConfig.getParameter<std::string>("tauHadIsoTag")),
  overlapCandTag_(iConfig.existsAs<edm::InputTag>("overlapCandTag") ? 
		  consumes<edm::RefVector<std::vector<T> > >(iConfig.getParameter<edm::InputTag>("overlapCandTag")) : edm::EDGetTokenT<edm::RefVector<std::vector<T> > >()),
  overlapCandTag1_(iConfig.existsAs<edm::InputTag>("overlapCandTag1") ?
                  consumes<edm::RefVector<std::vector<T> > >(iConfig.getParameter<edm::InputTag>("overlapCandTag1")) : edm::EDGetTokenT<edm::RefVector<std::vector<T> > >()),
  muons_(consumes<std::vector<pat::Muon> >(iConfig.getParameter<edm::InputTag>("muons"))),
  tauDiscriminatorTags_(iConfig.getParameter<std::vector<std::string> >("tauDiscriminatorTags")),
  passDiscriminator_(iConfig.getParameter<bool>("passDiscriminator")),
  pTMin_(iConfig.getParameter<double>("pTMin")),
  etaMax_(iConfig.getParameter<double>("etaMax")),
  isoMax_(iConfig.getParameter<double>("isoMax")),
  dR_(iConfig.getParameter<double>("dR")),
  minNumObjsToPassFilter_(iConfig.getParameter<unsigned int>("minNumObjsToPassFilter")),
  diTaudR_(iConfig.getParameter<double>("diTaudR")),
  outFileName_(iConfig.getParameter<std::string>("outFileName"))
//  histos1D_()
{

  produces<std::vector<pat::TauRef> >();
}

template<class T>
CustomTauSelector_MiniAOD<T>::~CustomTauSelector_MiniAOD()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called on each new Event  ------------
template<class T>
bool CustomTauSelector_MiniAOD<T>::filter(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  //create pointer to output collection
  std::auto_ptr<std::vector<pat::TauRef> > tauColl(new std::vector<pat::TauRef> );

  //get taus
  edm::Handle<pat::TauCollection> pTaus;
  if (tauTag_.isUninitialized()) {}
  else iEvent.getByToken(tauTag_, pTaus);

  //get base tau collection
  edm::Handle<pat::TauCollection> pBaseTaus;
  iEvent.getByToken(baseTauTag_, pBaseTaus);

  //get overlap candidates
  edm::Handle<edm::RefVector<std::vector<T> > > pOverlapCands;
  if (overlapCandTag_.isUninitialized()) {}
  else iEvent.getByToken(overlapCandTag_, pOverlapCands);

  edm::Handle<edm::RefVector<std::vector<T> > > pOverlapCands1;
  if (overlapCandTag1_.isUninitialized()) {}
  else iEvent.getByToken(overlapCandTag1_, pOverlapCands1);

  //fill STL container of pointers to overlap candidates
  std::vector<T*> overlapCandPtrs;
  if (pOverlapCands.isValid())
  {
    for (typename edm::RefVector<std::vector<T> >::const_iterator iOverlapCand = pOverlapCands->begin(); iOverlapCand != pOverlapCands->end(); ++iOverlapCand)
      overlapCandPtrs.push_back(const_cast<T*>(iOverlapCand->get()));
  }
  std::vector<T*> overlapCandPtrs1;
  if (pOverlapCands1.isValid())
  {
    for (typename edm::RefVector<std::vector<T> >::const_iterator iOverlapCand1 = pOverlapCands1->begin(); iOverlapCand1 != pOverlapCands1->end(); ++iOverlapCand1)
      overlapCandPtrs1.push_back(const_cast<T*>(iOverlapCand1->get()));
  }

  //get base muon collection
  edm::Handle<std::vector<pat::Muon> > pMuons;
  iEvent.getByToken(muons_, pMuons);

  //fill STL container with taus passing specified discriminators in specified eta and pT range
  std::vector<pat::TauRef> taus = pTaus.isValid() ? 
    Common::getPATTaus(pTaus, pBaseTaus, tauDiscriminatorTags_, tauHadIsoTag_, pTMin_, etaMax_, passDiscriminator_, isoMax_) : 
    Common::getPATTaus(pBaseTaus, tauDiscriminatorTags_, tauHadIsoTag_, pTMin_, etaMax_, passDiscriminator_, isoMax_);

  //loop over selected taus
  unsigned int nPassingTaus = 0;
  for (std::vector<pat::TauRef>::const_iterator iTau = taus.begin(); iTau != taus.end(); ++iTau) 
  {
    if (fabs((*iTau)->charge() ) == 1)
    {
//      //find the nearest overlap candidate to the tau
//      double minDR = 1234567891123.0;
//      const reco::Candidate* nearestMuon = NULL;
//      if (pOverlapCands.isValid())
//      {
//        for (typename edm::RefVector<std::vector<T> >::const_iterator iOverlapCand = pOverlapCands->begin(); iOverlapCand != pOverlapCands->end(); ++iOverlapCand)
//        {
//          //double dPhi = reco::deltaPhi(iOverlapCand->phi(), iTau->phi()), dEta = iTau->eta() - OverlapCand->eta(), dR = sqrt(dPhi*dPhi + dEta*dEta);
//          double dR = reco::deltaR(**iTau, **iOverlapCand);
//          if (dR  < minDR) 
//          {
//            nearestMuon = *iOverlapCand;
//            minDR = dR;
//          }// if dR  < minDR
//        }//for iOverlapCand
//      }//if pOverlapCands   
//      minDR = 1234567891123.0;
//      const reco::Candidate* nearestMuon1 = NULL;
//      if (pOverlapCands1.isValid())
//      {
//        for (typename edm::RefVector<std::vector<T> >::const_iterator iOverlapCand1 = pOverlapCands1->begin(); iOverlapCand1 != pOverlapCands1->end(); ++iOverlapCand1)
//        {
//          //double dPhi = reco::deltaPhi(iOverlapCand1->phi(), iTau->phi()), dEta = iTau->eta() - OverlapCand->eta(), dR = sqrt(dPhi*dPhi + dEta*dEta);
//          double dR = reco::deltaR(**iTau, **iOverlapCand1);
//          if (dR  < minDR) 
//          {
//            nearestMuon1 = *iOverlapCand1;
//            minDR = dR;
//          }// if dR  < minDR
//        }//for iOverlapCand1
//      }   

      //find the nearest overlap candidate to the tau
      int nearestMuonIndex = -1;
      int nearestMuonIndex1 = -1;
      const reco::Candidate* nearestMuon = Common::nearestObject(*iTau, overlapCandPtrs, nearestMuonIndex);
      const reco::Candidate* nearestMuon1 = Common::nearestObject(*iTau, overlapCandPtrs1, nearestMuonIndex1);

      //if tau doesn't overlap with overlap candidate (or no overlap checking requested)...
      if ((!(pOverlapCands.isValid()) || ((nearestMuon != NULL) && (reco::deltaR(**iTau, *nearestMuon) > dR_)))&&(!(pOverlapCands1.isValid()) || ((nearestMuon1 != NULL) && (reco::deltaR(**iTau, *nearestMuon1) > dR_))) ) 
      {
        for (std::vector<pat::Muon>::const_iterator iMu = pMuons->begin(); iMu != pMuons->end(); ++iMu)
        {
          double dR = reco::deltaR(**iTau, *iMu);
          if (dR < diTaudR_)
          {
            TauHadTauMudR_->Fill(dR );
            TauPt_->Fill((*iTau)->pt());
            tauColl->push_back(*iTau);
            ++nPassingTaus;
          }//if dR < diTaudR_
        }//for iMu
      }//if pOverlapCands
    }//if iTaucharge
  }
  iEvent.put(tauColl);

  //if not enough taus passing cuts were found in this event, stop processing
  NPassing_->Fill((nPassingTaus >= minNumObjsToPassFilter_));
  return (nPassingTaus >= minNumObjsToPassFilter_);
}

// ------------ method called once each job just before starting event loop  ------------
template<class T>
void CustomTauSelector_MiniAOD<T>::beginJob()
{
  out_ = new TFile(outFileName_.c_str(), "RECREATE");
  TauPt_ = new TH1F("TauPt", "TauPt",100,0,300);
  NPassing_ = new TH1F("nPassing","nPassing",2,0,2);
  TauHadTauMudR_ = new TH1F("TauHadTauMudR", "TauHadTauMudR", 100, 0,1.0);
}

// ------------ method called once each job just after ending the event loop  ------------
template<class T>
void CustomTauSelector_MiniAOD<T>::endJob() {
  out_->cd();

  TCanvas TauPtCanvas_("TauPtCanvas","",600,600);
  TCanvas NPassingCanvas_("NPassingCanvas","",600,600);
  TCanvas TauHadTauMudRCanvas_("TauHadTauMudRCanvas","",600,600);

  Common::draw1DHistograms(TauPtCanvas_, TauPt_);
  Common::draw1DHistograms(NPassingCanvas_, NPassing_);
  Common::draw1DHistograms(TauHadTauMudRCanvas_, TauHadTauMudR_);

  TauPtCanvas_.Write();
  NPassingCanvas_.Write();
  TauHadTauMudRCanvas_.Write();

  out_->Write();
  out_->Close();
}

// ------------ method called when starting to processes a run  ------------
template<class T>
bool CustomTauSelector_MiniAOD<T>::beginRun(edm::Run&, edm::EventSetup const&)
{ 
  return true;
}
 
// ------------ method called when ending the processing of a run  ------------
template<class T>
bool CustomTauSelector_MiniAOD<T>::endRun(edm::Run&, edm::EventSetup const&)
{
  return true;
}

// ------------ method called when starting to processes a luminosity block  ------------
template<class T>
bool CustomTauSelector_MiniAOD<T>::beginLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&)
{
  return true;
}

// ------------ method called when ending the processing of a luminosity block  ------------
template<class T>
bool CustomTauSelector_MiniAOD<T>::endLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&)
{
  return true;
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
template<class T>
void CustomTauSelector_MiniAOD<T>::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}
//define this as a plug-in
typedef CustomTauSelector_MiniAOD<reco::Muon> CustomTauSepFromMuonSelector_MiniAOD;
typedef CustomTauSelector_MiniAOD<reco::Photon> CustomTauSepFromPhotonSelector_MiniAOD;
DEFINE_FWK_MODULE(CustomTauSepFromMuonSelector_MiniAOD);
DEFINE_FWK_MODULE(CustomTauSepFromPhotonSelector_MiniAOD);

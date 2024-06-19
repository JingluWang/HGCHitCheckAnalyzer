//cmsRun test/hgchitcheckanalyzer_cfg.py #check cfg for options

#include <iostream>
#include <map>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESTransientHandle.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "DetectorDescription/Core/interface/DDCompactView.h"
#include "SimDataFormats/CaloHit/interface/PCaloHitContainer.h"
#include "SimDataFormats/CaloHit/interface/PCaloHit.h"
#include "DataFormats/ForwardDetId/interface/HGCalDetId.h"
#include "DataFormats/ForwardDetId/interface/HGCSiliconDetId.h"
#include "DataFormats/ForwardDetId/interface/HGCScintillatorDetId.h"
#include "DataFormats/HGCDigi/interface/HGCDigiCollections.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "Geometry/HGCalGeometry/interface/HGCalGeometry.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"

/**
   @short this is a simple structure to store combined information of geometry, simhits, digis and rechits
*/
struct CustomHGCALDetIdAccumulator {
  CustomHGCALDetIdAccumulator() :
        hasValidDetId(false), hasValidSimHit(false), hasValidDigi(false), hasValidRecHit(false) { }
  CustomHGCALDetIdAccumulator(bool vdid, bool vsim, bool vdigi, bool vrec) :
    hasValidDetId(vdid), hasValidSimHit(vsim), hasValidDigi(vdigi), hasValidRecHit(vrec) { }
  CustomHGCALDetIdAccumulator(const CustomHGCALDetIdAccumulator& o) :
    hasValidDetId(o.hasValidDetId), hasValidSimHit(o.hasValidSimHit), hasValidDigi(o.hasValidDigi), hasValidRecHit(o.hasValidRecHit) { }
  bool hasValidDetId, hasValidSimHit, hasValidDigi, hasValidRecHit;
};


/**
   @short this is an example analyzer to get started with the matching of SIM-DIGI-REC hits
 */
class HGCHitCheckAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources>
{
  
public:
  
  explicit HGCHitCheckAnalyzer( const edm::ParameterSet& );
  ~HGCHitCheckAnalyzer() {}

  void beginJob() override;
  virtual void analyze( const edm::Event&, const edm::EventSetup& );
  void endJob();
  
private:
  
  edm::EDGetTokenT<edm::PCaloHitContainer> simHitsCEE_,simHitsCEH_,simHitsCEHSci_;
  edm::EDGetTokenT<HGCalDigiCollection> digisCEE_,digisCEH_,digisCEHSci_;
  edm::EDGetTokenT<HGCRecHitCollection> hitsCEE_,hitsCEH_,hitsCEHSci_;
  edm::ESGetToken<CaloGeometry, CaloGeometryRecord> caloGeomToken_;

  // Hists
  TH1D *h_LayerEE011;
  TH1D *h_LayerEE111;
  TH1D *h_LayerEE100;
  TH1D *h_LayerEE101;

  TH1D *h_LayerHSi011;
  TH1D *h_LayerHSi111;
  TH1D *h_LayerHSi100;
  TH1D *h_LayerHSi101;

  TH1D *h_LayerHSc011;
  TH1D *h_LayerHSc111;
  TH1D *h_LayerHSc100;
  TH1D *h_LayerHSc101;

};
 



//
HGCHitCheckAnalyzer::HGCHitCheckAnalyzer( const edm::ParameterSet &iConfig ) 
  : simHitsCEE_( consumes<std::vector<PCaloHit>>(edm::InputTag("g4SimHits", "HGCHitsEE")) ),
    simHitsCEH_( consumes<std::vector<PCaloHit>>(edm::InputTag("g4SimHits", "HGCHitsHEfront")) ),
    simHitsCEHSci_( consumes<std::vector<PCaloHit>>(edm::InputTag("g4SimHits", "HGCHitsHEback")) ),
    digisCEE_( consumes<HGCalDigiCollection>(edm::InputTag("simHGCalUnsuppressedDigis","EE")) ),
    digisCEH_( consumes<HGCalDigiCollection>(edm::InputTag("simHGCalUnsuppressedDigis","HEfront")) ),
    digisCEHSci_( consumes<HGCalDigiCollection>(edm::InputTag("simHGCalUnsuppressedDigis","HEback")) ),
    hitsCEE_(consumes<HGCRecHitCollection>(edm::InputTag("HGCalRecHit","HGCEERecHits")) ),
    hitsCEH_(consumes<HGCRecHitCollection>(edm::InputTag("HGCalRecHit","HGCHEFRecHits")) ),
    hitsCEHSci_(consumes<HGCRecHitCollection>(edm::InputTag("HGCalRecHit","HGCHEBRecHits")) ),
    caloGeomToken_(esConsumes<CaloGeometry, CaloGeometryRecord>())
{   
}

//
void HGCHitCheckAnalyzer::endJob()
{
}

//
void HGCHitCheckAnalyzer::analyze( const edm::Event &iEvent, const edm::EventSetup &iSetup)
{

  //key is the detId, content is the accumulator of information for a given detId
  std::map<uint32_t,CustomHGCALDetIdAccumulator> hgcalDetIdMapper;
  
  //read geometry components for HGCAL
  // 0 - CE-E
  // 1 - CE-H Si
  // 2 - CE-H SiPM-on-tile
  edm::ESHandle<CaloGeometry> geom = iSetup.getHandle(caloGeomToken_);
  assert(geom.isValid());
  
  for(int i=0; i<3; i++) {

    //read the valid detIds from the geometry of the detector region
    auto detcode = (i==0? DetId::HGCalEE : (i==1? DetId::HGCalHSi : DetId::HGCalHSc));
    const HGCalGeometry *geo = static_cast<const HGCalGeometry*>(geom->getSubdetectorGeometry(detcode, ForwardSubdetector::ForwardEmpty));
    const std::vector<DetId>& validIds = geo->getValidDetIds();
    
    //get simHits, digis and recHits from the event
    edm::Handle<edm::PCaloHitContainer> simHits;
    edm::Handle<HGCalDigiCollection> digis;
    edm::Handle<HGCRecHitCollection> recHits;
    if(i==0) {
      iEvent.getByToken(simHitsCEE_, simHits);
      iEvent.getByToken(digisCEE_,   digis);
      iEvent.getByToken(hitsCEE_,    recHits);
    }
    if(i==1) {
      iEvent.getByToken(simHitsCEH_, simHits);
      iEvent.getByToken(digisCEH_,   digis);
      iEvent.getByToken(hitsCEH_,    recHits);
    }
    if(i==2) {
      iEvent.getByToken(simHitsCEHSci_, simHits);
      iEvent.getByToken(digisCEHSci_,  digis);
      iEvent.getByToken(hitsCEHSci_,   recHits);
    }
    assert(simHits.isValid());
    assert(digis.isValid());
    assert(recHits.isValid());
    
    std::cout << "@Sub-detector " << detcode
              << " #det-ids: " << validIds.size()
              << " #simHits: " << simHits->size()
              << " #digis: " << digis->size()
              << " #recHits: " << recHits->size()
              << std::endl;

//      HGCSiliconDetId(validIds.rawId());

    //flag valid detIds
    for(auto id : validIds) hgcalDetIdMapper[id.rawId()] = CustomHGCALDetIdAccumulator(true,false,false,false);

    //flag detIds with valid simhits
    for(auto sh : *simHits) {
      uint32_t rawid(sh.id());
      auto it=hgcalDetIdMapper.find(rawid);
      if(it==hgcalDetIdMapper.end()) {
        hgcalDetIdMapper[rawid] = CustomHGCALDetIdAccumulator(false,true,false,false);
      } else {
        it->second.hasValidSimHit=true;
      }
   }

    //flag detIds with valid digis
    for(auto d : *digis) {
      uint32_t rawid(d.id().rawId());
      auto it=hgcalDetIdMapper.find(rawid);
      if(it==hgcalDetIdMapper.end()) {        
        hgcalDetIdMapper[rawid] = CustomHGCALDetIdAccumulator(false,false,true,false);
      } else {
        it->second.hasValidDigi=true;
      }
    }

    //flag detIds with valid rechits
    for(auto r: *recHits) {
      uint32_t rawid(r.id().rawId());
      auto it=hgcalDetIdMapper.find(rawid);
      if(it==hgcalDetIdMapper.end()) {
        hgcalDetIdMapper[rawid] = CustomHGCALDetIdAccumulator(false,false,false,true);
      } else {
        it->second.hasValidRecHit=true;
      }
    }
  }


  //analyze the accumulators, maybe fill some plots etc. :)
  std::cout << "size of 'hgcalDetIdMapper': " << hgcalDetIdMapper.size() << std::endl;
 

  ///////////////////////////////////////// 
  // Count Sim-Digi-Rec Hits
  // ---------------------------
  // Case011 -- Noise (expected)
  // Case111 -- Signal Hits
  // Case100 -- Error
  // Case101 -- Unexpected
  int N_EE011 = 0;// 1-1. Silicon - E
  int N_EE111 = 0;
  int N_EE100 = 0;
  int N_EE101 = 0;

  int N_HSi011 = 0;// 1-2. Silicon - H
  int N_HSi111 = 0;
  int N_HSi100 = 0;
  int N_HSi101 = 0;

  int N_HSc011 = 0;// 2. Scintillator
  int N_HSc111 = 0;
  int N_HSc100 = 0;
  int N_HSc101 = 0;

  // Loop through Mapper
  auto it = hgcalDetIdMapper.begin();
  while ( it != hgcalDetIdMapper.end() ) {
 
      // -----------------------------------------------------------------
      // Initialize DetId with rawid --> Silicon or Scintillator
      auto mapdetid = DetId( it->first );
      bool isEE     = mapdetid.det() == DetId::HGCalEE;  // Silicon - CEE
      bool isHSi    = mapdetid.det() == DetId::HGCalHSi; // Silicon - CEH
      bool isHSc    = mapdetid.det() == DetId::HGCalHSc; // Scintillator

      // -----------------------------------------------------------------
      // Get hits
      // Sim
      bool isSim = it->second.hasValidSimHit;
      // Digi
      bool isDigi = it->second.hasValidDigi;
      // Rec
      bool isRec = it->second.hasValidRecHit;

      // Sim-Digi-Rec cases
      bool Case011 = !isSim &&  isDigi &&  isRec;// Noise (expected)
      bool Case111 =  isSim &&  isDigi &&  isRec;// Signal Hits
      bool Case100 =  isSim && !isDigi && !isRec;// Error
      bool Case101 =  isSim && !isDigi &&  isRec;// Unexpected

      // -----------------------------------------------------------------
      // -----------------------------------------------------------------
      // 1. Silicon
      if ( isEE || isHSi ) {
          // Initialize HGCSiliconDetId with rawid
          auto Silicon = HGCSiliconDetId( it->first );

          // 1-1 E
          if ( isEE ) {
              if ( Case011 ) { N_EE011++; h_LayerEE011->Fill( Silicon.layer() ); }
              if ( Case111 ) { N_EE111++; h_LayerEE111->Fill( Silicon.layer() ); }
              if ( Case100 ) { N_EE100++; h_LayerEE100->Fill( Silicon.layer() ); }
              if ( Case101 ) { N_EE101++; h_LayerEE101->Fill( Silicon.layer() ); }
          }

          // 1-2 H
          if ( isHSi ) {
              if ( Case011 ) { N_HSi011++; h_LayerHSi011->Fill( Silicon.layer() ); }
              if ( Case111 ) { N_HSi111++; h_LayerHSi111->Fill( Silicon.layer() ); }
              if ( Case100 ) { N_HSi100++; h_LayerHSi100->Fill( Silicon.layer() ); }
              if ( Case101 ) { N_HSi101++; h_LayerHSi101->Fill( Silicon.layer() ); }
          }
      }

      // 2. Scintillator
      if ( isHSc ) {
          // Initialize ScintillatorDetId with rawid
          auto Scintillator = HGCScintillatorDetId( it->first );

          if ( Case011 ) { N_HSc011++; h_LayerHSc011->Fill( Scintillator.layer() ); }
          if ( Case111 ) { N_HSc111++; h_LayerHSc111->Fill( Scintillator.layer() ); }
          if ( Case100 ) { N_HSc100++; h_LayerHSc100->Fill( Scintillator.layer() ); }
          if ( Case101 ) { N_HSc101++; h_LayerHSc101->Fill( Scintillator.layer() ); }

      }

/*
      if ( it->second.hasValidSimHit || it->second.hasValidDigi || it->second.hasValidRecHit ) {
        N_anyOf3++;

        auto mapdetid = DetId(it->first);
      // 1. SiliconDetId
      if ( mapdetid.det() == DetId::HGCalEE || mapdetid.det() == DetId::HGCalHSi ) {
          auto Silicon = HGCSiliconDetId( it->first );
          //std::cout << "Silicon.layer=" << Silicon.layer() << std::endl;

      }

      // 2. ScintillatorDetId
      if ( mapdetid.det() == DetId::HGCalHSc ) {
          auto Scintillator = HGCScintillatorDetId( it->first );
      }
*/
/*
// outputs(Jun8)
      std:: cout << "key(detId): " << it->first 
                 << ",    DetId: " << it->second.hasValidDetId
                 << ",    SimHit: " << it->second.hasValidSimHit
                 << ",    Digi: " << it->second.hasValidDigi
                 << ",    RecHit: " << it->second.hasValidRecHit
                 << std::endl;
*/

      ++it;
  }
  //std::cout << "N_anyOf3=" << N_anyOf3 << ", N_none=" << N_none << std::endl;
}

// ------------ method called once each job just before starting event loop  ------------
void HGCHitCheckAnalyzer::beginJob() {
    edm::Service<TFileService> fs;

    h_LayerEE011 = fs->make<TH1D>("Layer_EE011", "", 30, 0.5, 30.5);
    h_LayerEE111 = fs->make<TH1D>("Layer_EE111", "", 30, 0.5, 30.5);
    h_LayerEE100 = fs->make<TH1D>("Layer_EE100", "", 30, 0.5, 30.5);
    h_LayerEE101 = fs->make<TH1D>("Layer_EE101", "", 30, 0.5, 30.5);

    h_LayerHSi011 = fs->make<TH1D>("Layer_HSi011", "", 30, 0.5, 30.5);
    h_LayerHSi111 = fs->make<TH1D>("Layer_HSi111", "", 30, 0.5, 30.5);
    h_LayerHSi100 = fs->make<TH1D>("Layer_HSi100", "", 30, 0.5, 30.5);
    h_LayerHSi101 = fs->make<TH1D>("Layer_HSi101", "", 30, 0.5, 30.5);

    h_LayerHSc011 = fs->make<TH1D>("Layer_HSc011", "", 30, 0.5, 30.5);
    h_LayerHSc111 = fs->make<TH1D>("Layer_HSc111", "", 30, 0.5, 30.5);
    h_LayerHSc100 = fs->make<TH1D>("Layer_HSc100", "", 30, 0.5, 30.5);
    h_LayerHSc101 = fs->make<TH1D>("Layer_HSc101", "", 30, 0.5, 30.5);
}

//define this as a plug-in
DEFINE_FWK_MODULE(HGCHitCheckAnalyzer);

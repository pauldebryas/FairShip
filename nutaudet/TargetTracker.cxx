//
//  TargetTracker.cxx
//  
//
//  Created by Annarita Buonaura on 21/10/15.
//
//

#include "TargetTracker.h"

#include "TTPoint.h"

#include "TGeoManager.h"
#include "FairRun.h"                    // for FairRun
#include "FairRuntimeDb.h"              // for FairRuntimeDb
#include <iosfwd>                    // for ostream
#include "TList.h"                      // for TListIter, TList (ptr only)
#include "TObjArray.h"                  // for TObjArray
#include "TString.h"                    // for TString

#include "TClonesArray.h"
#include "TVirtualMC.h"

#include "TGeoBBox.h"
#include "TGeoTrd1.h"
#include "TGeoCompositeShape.h"
#include "TGeoTube.h"
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoTrd1.h"
#include "TGeoArb8.h"

#include "TParticle.h"
#include "TParticlePDG.h"
#include "TParticleClassPDG.h"
#include "TVirtualMCStack.h"

#include "FairVolume.h"
#include "FairGeoVolume.h"
#include "FairGeoNode.h"
#include "FairRootManager.h"
#include "FairGeoLoader.h"
#include "FairGeoInterface.h"
#include "FairGeoTransform.h"
#include "FairGeoMedia.h"
#include "FairGeoMedium.h"
#include "FairGeoBuilder.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"

#include "ShipDetectorList.h"
#include "ShipUnit.h"
#include "ShipStack.h"

#include "TGeoUniformMagField.h"
#include <stddef.h>                     // for NULL
#include <iostream>                     // for operator<<, basic_ostream,etc
#include <string.h>

using std::cout;
using std::endl;

using namespace ShipUnit;

TargetTracker::TargetTracker()
: FairDetector("TargetTracker", "",kTRUE),
  fTrackID(-1),
fVolumeID(-1),
fPos(),
fMom(),
fTime(-1.),
fLength(-1.),
fELoss(-1),
fTTPointCollection(new TClonesArray("TTPoint"))
{
}

TargetTracker::TargetTracker(const char* name, Bool_t Active,const char* Title)
: FairDetector(name, true, ktauTT),
  fTrackID(-1),
fVolumeID(-1),
fPos(),
fMom(),
fTime(-1.),
fLength(-1.),
fELoss(-1),
fTTPointCollection(new TClonesArray("TTPoint"))
{
}

TargetTracker::~TargetTracker()
{
    if (fTTPointCollection) {
        fTTPointCollection->Delete();
        delete fTTPointCollection;
    }
}

void TargetTracker::Initialize()
{
    FairDetector::Initialize();
}

// -----   Private method InitMedium
Int_t TargetTracker::InitMedium(const char* name)
{
    static FairGeoLoader *geoLoad=FairGeoLoader::Instance();
    static FairGeoInterface *geoFace=geoLoad->getGeoInterface();
    static FairGeoMedia *media=geoFace->getMedia();
    static FairGeoBuilder *geoBuild=geoLoad->getGeoBuilder();
    
    FairGeoMedium *ShipMedium=media->getMedium(name);
    
    if (!ShipMedium)
    {
        Fatal("InitMedium","Material %s not defined in media file.", name);
        return -1111;
    }
    TGeoMedium* medium=gGeoManager->GetMedium(name);
    if (medium!=NULL)
        return ShipMedium->getMediumIndex();
    return geoBuild->createMedium(ShipMedium);
}

void TargetTracker::SetTargetTrackerParam(Double_t TTX, Double_t TTY, Double_t TTZ)
{
    TTrackerX = TTX;
    TTrackerY = TTY;
    TTrackerZ = TTZ;
}

void TargetTracker::SetBrickParam(Double_t CellW)
{
  CellWidth = CellW;
}

void TargetTracker::SetTotZDimension(Double_t Zdim)
{
  ZDimension = Zdim;
}

void TargetTracker::SetTotXDimension(Double_t Xdim)
{
  XDimension = Xdim;
}

void TargetTracker::SetThicnessRibbon(Double_t Thicknessribbon)
{
  ZRibbon = Thicknessribbon;
}

void TargetTracker::SetNumberTT(Int_t n)
{
  fNTT =n;
}

void TargetTracker::SetDesign(Int_t Design)
{
  fDesign = Design;
}

void TargetTracker::SetNumberXChannel(Int_t NumberOfXChannel)
{
  chX = NumberOfXChannel;
}

void TargetTracker::SetNumberYChannel(Int_t NumberOfYChannel)
{
  chY = NumberOfYChannel;
}

void TargetTracker::ConstructGeometry()
{
  TGeoVolume *volTarget=gGeoManager->GetVolume("volTarget");

  InitMedium("TTmedium");
  TGeoMedium *mediumTT =gGeoManager->GetMedium("TTmedium");

  InitMedium("vacuum");
  TGeoMedium *vacuum =gGeoManager->GetMedium("vacuum");

  TGeoBBox *TTBox = new TGeoBBox("TTBox",TTrackerX/2, TTrackerY/2, TTrackerZ/2);
  TGeoVolume *volTT = new TGeoVolume("TargetTracker",TTBox,vacuum);
  volTT->SetLineColor(kBlue);

  TGeoBBox *ribbonX = new TGeoBBox("ribbonX",TTrackerX/2, TTrackerY/2, ZRibbon/2);
  TGeoVolume *volribbonX = new TGeoVolume("RibbonXVolume",ribbonX,vacuum);
  volribbonX->SetLineColor(kBlue);

  TGeoBBox *ribbonY = new TGeoBBox("ribbonY",TTrackerX/2, TTrackerY/2, ZRibbon/2);
  TGeoVolume *volribbonY = new TGeoVolume("RibbonYVolume",ribbonY,vacuum);
  volribbonY->SetLineColor(kBlue);

  TGeoBBox *Honneycomb = new TGeoBBox("Honneycomb",TTrackerX/2, TTrackerY/2, TTrackerZ/2-2*ZRibbon/2);
  TGeoVolume *volhonneycomb = new TGeoVolume("HonneycombVolume",Honneycomb,vacuum);
  volhonneycomb->SetLineColor(kBlue);

  TGeoBBox *channelX = new TGeoBBox("channelX",TTrackerX/(2*chX), TTrackerY/2, ZRibbon/2);
  TGeoVolume *volchannelX = new TGeoVolume("ChannelXVolume",channelX,mediumTT);
  volchannelX->SetLineColor(kGreen);
  AddSensitiveVolume(volchannelX);

  TGeoBBox *channelY = new TGeoBBox("channelY",TTrackerX/2, TTrackerY/(2*chY), ZRibbon/2);
  TGeoVolume *volchannelY = new TGeoVolume("ChannelYVolume",channelY,mediumTT);
  volchannelY->SetLineColor(kRed);
  AddSensitiveVolume(volchannelY);

  for(int m = 0; m < chX; m++)
    {
        volribbonX->AddNode(volchannelX,m,new TGeoTranslation(-TTrackerX/2+TTrackerX/(2*chX)+m*TTrackerX/chX,0,0));
    }

  for(int m = 0; m < chY; m++)
    {
        volribbonY->AddNode(volchannelY,m,new TGeoTranslation(0,-TTrackerY/2+TTrackerY/(2*chY)+m*TTrackerY/chY,0));
    }

  volTT->AddNode(volribbonX,1,new TGeoTranslation(0,0,-TTrackerZ/2+ZRibbon/2));
  volTT->AddNode(volribbonY,2,new TGeoTranslation(0,0,TTrackerZ/2-ZRibbon/2));
  volTT->AddNode(volhonneycomb,3,new TGeoTranslation(0,0,0)); 
    
  for(int l = 0; l < fNTT; l++)
    {
        volTarget->AddNode(volTT,l,new TGeoTranslation(0,0, -ZDimension/2 + TTrackerZ/2 + l*(TTrackerZ +CellWidth)));
    }

}

Bool_t TargetTracker::ProcessHits(FairVolume* vol)
{
    /** This method is called from the MC stepping */
    //Set parameters at entrance of volume. Reset ELoss.
    if ( gMC->IsTrackEntering() ) {
        fELoss  = 0.;
        fTime   = gMC->TrackTime() * 1.0e09;
        fLength = gMC->TrackLength();
        gMC->TrackPosition(fPos);
        gMC->TrackMomentum(fMom);
    }
    // Sum energy loss for all steps in the active volume
    fELoss += gMC->Edep();
    
    // Create muonPoint at exit of active volume
    if ( gMC->IsTrackExiting()    ||
        gMC->IsTrackStop()       ||
        gMC->IsTrackDisappeared()   ) {
        fTrackID  = gMC->GetStack()->GetCurrentTrackNumber();
        //Int_t fTrackID  = gMC->GetStack()->GetCurrentTrackNumber();
        gMC->CurrentVolID(fVolumeID);
	//gGeoManager->PrintOverlaps();
	
	//cout<< "detID = " << detID << endl;
	Int_t MaxLevel = gGeoManager->GetLevel();
	const Int_t MaxL = MaxLevel;
       	//cout << gMC->CurrentVolPath()<< endl;

	const char *name;
	
	Double_t zEnd = 0, zStart =0;

	
	if (fELoss == 0. ) { return kFALSE; }
        TParticle* p=gMC->GetStack()->GetCurrentTrack();
	Int_t fMotherID =p->GetFirstMother();
	Int_t pdgCode = p->GetPdgCode();

        TLorentzVector Pos; 
        gMC->TrackPosition(Pos);
        Double_t xmean = (fPos.X()+Pos.X())/2. ;      
        Double_t ymean = (fPos.Y()+Pos.Y())/2. ;      
        Double_t zmean = (fPos.Z()+Pos.Z())/2. ;     
        


        cout << "| fVolumeID = " << fVolumeID << "   | fMotherID = " << fMotherID <<  endl;



	AddHit(fTrackID,fVolumeID, TVector3(xmean, ymean,  zmean),
               TVector3(fMom.Px(), fMom.Py(), fMom.Pz()), fTime, fLength,
               fELoss, pdgCode);
	
        // Increment number of muon det points in TParticle
        ShipStack* stack = (ShipStack*) gMC->GetStack();
        stack->AddPoint(ktauTT);
    }
    
    return kTRUE;
}


void TargetTracker::DecodeTTID(Int_t detID, Int_t &NTT)
{
  NTT = detID;
}


void TargetTracker::EndOfEvent()
{
    fTTPointCollection->Clear();
}


void TargetTracker::Register()
{
    
    /** This will create a branch in the output tree called
     TargetPoint, setting the last parameter to kFALSE means:
     this collection will not be written to the file, it will exist
     only during the simulation.
     */
    
    FairRootManager::Instance()->Register("TTPoint", "TargetTracker",
                                          fTTPointCollection, kTRUE);
}

TClonesArray* TargetTracker::GetCollection(Int_t iColl) const
{
    if (iColl == 0) { return fTTPointCollection; }
    else { return NULL; }
}

void TargetTracker::Reset()
{
    fTTPointCollection->Clear();
}


TTPoint* TargetTracker::AddHit(Int_t trackID,Int_t detID,
                           TVector3 pos, TVector3 mom,
                           Double_t time, Double_t length,
			    Double_t eLoss, Int_t pdgCode)
{
    TClonesArray& clref = *fTTPointCollection;
    Int_t size = clref.GetEntriesFast();
    //cout << "brick hit called"<< pos.z()<<endl;
    return new(clref[size]) TTPoint(trackID,detID, pos, mom,
					time, length, eLoss, pdgCode);
}

ClassImp(TargetTracker)


#include "SpiderLeg.cpp"
#include <iostream>

#define Knee_Up_Base 60
#define Knee_Down_Base 45
#define HipF_Base -20
#define HipM_Base 0
#define HipB_Base 20
#define Ankle_Base 45

class Spider {
	typedef enum{
		LEG_RF,
		LEG_RM,
		LEG_RB,
		LEG_LF,
		LEG_LM,
		LEG_LB,
		LEG_NUM
	} LEG_ID;

	typedef enum{
		TRIPOD1,    //RF LM RB
		TRIPOD2,	//LF RM LB
		TRIPOD_NUM
	} TRIPOD_ID;

	typedef enum{
		FWD,
		BACK
	} DIR;

	SpiderLeg *m_szLeg[LEG_NUM];
  
	TRIPOD_ID lastStep;
	DIR lastDir;
	MMap* _mmio;

public:

	Spider()
	{
		_mmio = new MMap();		
		int szMotorID[] = {
			/* LEG_RF */ 0, 1, 2,
			/* LEG_RM */ 3, 4, 5,
			/* LEG_RB */ 6, 7, 8,
			/* LEG_LF */ 9, 10, 11,
			/* LEG_LM */ 12, 13, 14,
			/* LEG_LB */ 15, 16, 17};

		for (int i = 0; i < LEG_NUM; i++)
		{
			// Reverse the angles on all of the RHS motors
			m_szLeg[i] = new SpiderLeg(_mmio, szMotorID[i * 3],
                                       szMotorID[i * 3 + 1], szMotorID[i * 3 + 2],
                                       i == LEG_RF || i == LEG_RB || i == LEG_RM);
		}
		lastStep = TRIPOD2;
		lastDir = FWD;
	}

	~Spider() { for (int i = 0; i < LEG_NUM; i++)  delete m_szLeg[i]; }

	// Retracts all legs so the spider can commence Standup
	void Init()
	{
		//list of base position for hip corresponding to index
		float fszJoin0Angle[] = {HipF_Base, 0, HipB_Base,
								HipF_Base, 0, HipB_Base};
		
		//// Init -- The servo angle needs to be explicitly set to 0.0 to enable.
		for (int i = 0; i < LEG_NUM; i++){
			m_szLeg[i]->MoveJoint(SpiderLeg::Hip, 0.0); //No WaitReady() after this, want to enable and skip the action
			m_szLeg[i]->MoveJoint(SpiderLeg::Hip, fszJoin0Angle[i]); //Sets each hip to its base position
			WaitReady();
			m_szLeg[i]->MoveJoint(SpiderLeg::Knee, 0.0);
			WaitReady();
			m_szLeg[i]->MoveJoint(SpiderLeg::Ankle, 0.0);
			m_szLeg[i]->MoveJoint(SpiderLeg::Ankle, 45.0); //Set ankles to its halfway point
			WaitReady();
		}
		WaitReady();
	}
	
	//Waits until the spider is confirmed to be ready to return true
	bool WaitReady()
	{
		bool bReady = false;
		while (!bReady) bReady = IsReady();
		return bReady;
	}

	//Calls SpiderLeg to check if any legs aren't ready, if nothing changes the spider is ready
	bool IsReady()
	{
		bool bReady = true;
		for (int i = 0; i < LEG_NUM && bReady; i++)
			if (!m_szLeg[i]->IsReady()) bReady = false;
		return bReady;
	}
	
	//Moves the spider forward with one step from each tripod
	void MoveForward()
	{
		if ((lastStep == TRIPOD2 && lastDir == FWD) || (lastStep == TRIPOD1 && lastDir == BACK)){
			MoveTripod(TRIPOD1, SpiderLeg::Knee, Knee_Up_Base, Knee_Up_Base, Knee_Up_Base);
			WaitReady();
			MoveTripod(TRIPOD1, SpiderLeg::Hip, HipF_Base + 20, HipM_Base + 20, HipB_Base + 20);
			MoveTripod(TRIPOD2, SpiderLeg::Hip, HipF_Base - 20, HipM_Base - 20, HipB_Base - 20);
			WaitReady();
			MoveTripod(TRIPOD1, SpiderLeg::Knee, Knee_Down_Base, Knee_Down_Base, Knee_Down_Base);
			WaitReady();
			lastStep = TRIPOD1;
		}else{
			MoveTripod(TRIPOD2, SpiderLeg::Knee, Knee_Up_Base, Knee_Up_Base, Knee_Up_Base);
			WaitReady();
			MoveTripod(TRIPOD1, SpiderLeg::Hip, HipF_Base - 20, HipM_Base - 20, HipB_Base - 20);
			MoveTripod(TRIPOD2, SpiderLeg::Hip, HipF_Base + 20, HipM_Base + 20, HipB_Base + 20);
			WaitReady();
			MoveTripod(TRIPOD2, SpiderLeg::Knee, Knee_Down_Base, Knee_Down_Base, Knee_Down_Base);
			WaitReady();
			lastStep = TRIPOD2;
		}
		lastDir = FWD;
	}
	
	//Moves the spider backward with one step from each tripod
	void MoveBackward()
	{
		if ((lastStep == TRIPOD2 && lastDir == BACK) || (lastStep == TRIPOD1 && lastDir == FWD)){ //Flip these last step directions from forward
			MoveTripod(TRIPOD1, SpiderLeg::Knee, Knee_Up_Base, Knee_Up_Base, Knee_Up_Base);
			WaitReady();
			MoveTripod(TRIPOD1, SpiderLeg::Hip, HipF_Base - 20, HipM_Base - 20, HipB_Base - 20); //Flip the sign here
			MoveTripod(TRIPOD2, SpiderLeg::Hip, HipF_Base + 20, HipM_Base + 20, HipB_Base + 20); // and here,
			WaitReady();
			MoveTripod(TRIPOD1, SpiderLeg::Knee, Knee_Down_Base, Knee_Down_Base, Knee_Down_Base);
			WaitReady();
			lastStep = TRIPOD1;
		}else{
			MoveTripod(TRIPOD2, SpiderLeg::Knee, Knee_Up_Base, Knee_Up_Base, Knee_Up_Base);
			WaitReady();
			MoveTripod(TRIPOD1, SpiderLeg::Hip, HipF_Base + 20, HipM_Base + 20, HipB_Base + 20); // and here,
			MoveTripod(TRIPOD2, SpiderLeg::Hip, HipF_Base - 20, HipM_Base - 20, HipB_Base - 20); // and here
			WaitReady();
			MoveTripod(TRIPOD2, SpiderLeg::Knee, Knee_Down_Base, Knee_Down_Base, Knee_Down_Base);
			WaitReady();
			lastStep = TRIPOD2;
		}
		lastDir = BACK; //Flip last direction to reflect backwards movement
	}
	
	//Turns the spider 6 degrees to the right
	void Right(){
		//Return TRIPOD1 to base position
		MoveTripod(TRIPOD1, SpiderLeg::Knee, Knee_Up_Base, Knee_Up_Base, Knee_Up_Base);
		WaitReady();
		MoveTripod(TRIPOD1, SpiderLeg::Hip, HipF_Base, HipM_Base, HipB_Base);
		WaitReady();
		MoveTripod(TRIPOD1, SpiderLeg::Knee, Knee_Down_Base, Knee_Down_Base, Knee_Down_Base);
		//Return TRIPOD2 to base position
		MoveTripod(TRIPOD2, SpiderLeg::Knee, Knee_Up_Base, Knee_Up_Base, Knee_Up_Base);
		WaitReady();
		MoveTripod(TRIPOD2, SpiderLeg::Hip, HipF_Base, HipM_Base, HipB_Base);
		WaitReady();
		MoveTripod(TRIPOD2, SpiderLeg::Knee, Knee_Down_Base, Knee_Down_Base, Knee_Down_Base);
		WaitReady();
		
		
		MoveTripod(TRIPOD1, SpiderLeg::Knee, Knee_Up_Base, Knee_Up_Base, Knee_Up_Base); //Lift TRIPOD1 legs
		WaitReady();
		MoveTripod(TRIPOD1, SpiderLeg::Hip, HipF_Base + 20, HipM_Base - 20, HipB_Base + 20); //TRIPOD1 moves clockwise
		MoveTripod(TRIPOD2, SpiderLeg::Hip, HipF_Base - 20, HipM_Base + 20, HipB_Base - 20);//TRIPOD2 moves counter-clockwise
		WaitReady();
		MoveTripod(TRIPOD1, SpiderLeg::Knee, Knee_Down_Base, Knee_Down_Base, Knee_Down_Base); //Lower TRIPOD1 legs
		WaitReady();
		
		//Return TRIPOD1 to base position
		MoveTripod(TRIPOD1, SpiderLeg::Knee, Knee_Up_Base, Knee_Up_Base, Knee_Up_Base);
		WaitReady();
		MoveTripod(TRIPOD1, SpiderLeg::Hip, HipF_Base, HipM_Base, HipB_Base);
		WaitReady();
		MoveTripod(TRIPOD1, SpiderLeg::Knee, Knee_Down_Base, Knee_Down_Base, Knee_Down_Base);
		WaitReady();
	}
	
	//Turns the spider 6 degrees to the left
	void Left(){
		//Return TRIPOD1 to base position
		MoveTripod(TRIPOD1, SpiderLeg::Knee, Knee_Up_Base, Knee_Up_Base, Knee_Up_Base);
		WaitReady();
		MoveTripod(TRIPOD1, SpiderLeg::Hip, HipF_Base, HipM_Base, HipB_Base);
		WaitReady();
		MoveTripod(TRIPOD1, SpiderLeg::Knee, Knee_Down_Base, Knee_Down_Base, Knee_Down_Base);
		//Return TRIPOD2 to base position
		MoveTripod(TRIPOD2, SpiderLeg::Knee, Knee_Up_Base, Knee_Up_Base, Knee_Up_Base);
		WaitReady();
		MoveTripod(TRIPOD2, SpiderLeg::Hip, HipF_Base, HipM_Base, HipB_Base);
		WaitReady();
		MoveTripod(TRIPOD2, SpiderLeg::Knee, Knee_Down_Base, Knee_Down_Base, Knee_Down_Base);
		WaitReady();
		
		
		MoveTripod(TRIPOD1, SpiderLeg::Knee, Knee_Up_Base, Knee_Up_Base, Knee_Up_Base); //Lift TRIPOD1 legs
		WaitReady();
		MoveTripod(TRIPOD2, SpiderLeg::Hip, HipF_Base + 20, HipM_Base - 20, HipB_Base + 20); //TRIPOD2 moves clockwise
		MoveTripod(TRIPOD1, SpiderLeg::Hip, HipF_Base - 20, HipM_Base + 20, HipB_Base - 20);//TRIPOD1 moves counter-clockwise
		WaitReady();
		MoveTripod(TRIPOD1, SpiderLeg::Knee, Knee_Down_Base, Knee_Down_Base, Knee_Down_Base); //Lower TRIPOD1 legs
		WaitReady();
		
		//Return TRIPOD2 to base position
		MoveTripod(TRIPOD2, SpiderLeg::Knee, Knee_Up_Base, Knee_Up_Base, Knee_Up_Base);
		WaitReady();
		MoveTripod(TRIPOD2, SpiderLeg::Hip, HipF_Base, HipM_Base, HipB_Base);
		WaitReady();
		MoveTripod(TRIPOD2, SpiderLeg::Knee, Knee_Down_Base, Knee_Down_Base, Knee_Down_Base);
		WaitReady();
	}

	//Moves a specified joint on all three legs of a tripod to the 3 corresponding angles
	void MoveTripod(TRIPOD_ID Tripod, SpiderLeg::JOINT_ID Joint, float AngleF, float AngleM, float AngleB)
	{
		if (Tripod == 0){
			m_szLeg[LEG_RF]->MoveJoint(Joint, AngleF);
			m_szLeg[LEG_LM]->MoveJoint(Joint, AngleM);
			m_szLeg[LEG_RB]->MoveJoint(Joint, AngleB);
		}
		else{
			m_szLeg[LEG_LF]->MoveJoint(Joint, AngleF);
			m_szLeg[LEG_RM]->MoveJoint(Joint, AngleM);
			m_szLeg[LEG_LB]->MoveJoint(Joint, AngleB);
		}
	}

	//Brings the spider from its initialized position to a standing position so it can move freely
	void Standup()
	{
		bool bSuccess;
		bSuccess = WaitReady();

		//// Stand up  -- Adjust Knee ankle
		float KneeAngle = 90;
		while (bSuccess && KneeAngle >= 45.0) //Moves knee until it is at standing angle
		{
			MoveTripod(TRIPOD1, SpiderLeg :: Knee, KneeAngle, KneeAngle, KneeAngle); //Set both tripods at the same time
			MoveTripod(TRIPOD2, SpiderLeg :: Knee, KneeAngle, KneeAngle, KneeAngle);
			bSuccess = WaitReady();
			KneeAngle -= 15.0; // changed from -5.0 to increase movement speed
		}
		if (bSuccess) Reset();
	}
	
	//Makes sure all leg joints are in their base position before taking commands
	void Reset()
	{
		float fszJoin0Angle[] = {HipF_Base, 0, HipB_Base,
								 HipF_Base, 0, HipB_Base};

		////////////////////////
		////Reset Hip Knee ankle
		for (int i = 0; i < LEG_NUM - 3; i++)
		{
			m_szLeg[i]->MoveJoint(SpiderLeg::Knee, Knee_Up_Base);
			m_szLeg[LEG_NUM - i - 1]->MoveJoint(SpiderLeg::Knee, Knee_Up_Base);
			m_szLeg[i]->MoveJoint(SpiderLeg::Hip, fszJoin0Angle[i]);
			m_szLeg[LEG_NUM - i - 1]->MoveJoint(SpiderLeg::Hip, fszJoin0Angle[LEG_NUM - i - 1]);
			m_szLeg[i]->MoveJoint(SpiderLeg::Ankle, Ankle_Base);
			m_szLeg[LEG_NUM - i - 1]->MoveJoint(SpiderLeg::Ankle, Ankle_Base);
			WaitReady();
			m_szLeg[i]->MoveJoint(SpiderLeg::Knee, Knee_Down_Base);
			m_szLeg[LEG_NUM - i - 1]->MoveJoint(SpiderLeg::Knee, Knee_Down_Base);
			WaitReady();
		}
	}

};
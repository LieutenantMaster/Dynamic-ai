class Location_Trigger: CylinderTrigger
{       

    float Spatial_TriggerRadius, Spatial_Chance;
    float Spatial_Timer = 60000;
    string Spatial_Name, Spatial_ZoneLoadout, Spatial_Faction;
    int Spatial_MinCount, Spatial_MaxCount, Spatial_HuntMode, SpawnCount, Spatial_Lootable, i_PlayerCount;
    vector Spatial_TriggerPosition, Spatial_SpawnPosition;

    ref Spatial_Groups m_Spatial_Groups;
    eAISpatialPatrol dynPatrol;
    bool Spatial_TimerCheck;

    void Location_Trigger(){
      GetSpatialSettings().PullRef(m_Spatial_Groups);   
    }

    void Spatial_SetData(string a, float b, string c, int d, int e, int f, string g, vector h, vector i, int j, float k, float l){
      Spatial_Name = a;
      Spatial_TriggerRadius = b;
      Spatial_ZoneLoadout = c;
      Spatial_MinCount = d;
      Spatial_MaxCount = e;
      Spatial_HuntMode = f;
      Spatial_Faction = g;
      Spatial_TriggerPosition = h;
      Spatial_SpawnPosition = i;
      Spatial_Lootable = j;
      Spatial_Timer = k;
      Spatial_Chance = l;
    }

    void SpawnCheck(){
      if (Spatial_TimerCheck) return;
      i_PlayerCount = m_insiders.Count();
        if (dynPatrol) return;
        if (i_PlayerCount < 1) return;
        if (Spatial_Chance < Math.RandomFloat(0.0, 1.0)) return;

      SpawnCount = Math.RandomIntInclusive(Spatial_MinCount, Spatial_MaxCount);
      Spatial_Spawn(SpawnCount, Spatial_Faction, Spatial_ZoneLoadout, Spatial_Name, Spatial_Lootable);
      Spatial_TimerCheck = true;
      GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.Spatial_timer, Spatial_Timer, false);
    }

    void Spatial_timer(){
      Spatial_TimerCheck = false;
    }

    override void Enter(TriggerInsider insider){
      super.Enter(insider);
      SpawnCheck();
    }
        
    override void Leave(TriggerInsider insider){
      super.Leave(insider);
    }
    
    override protected bool CanAddObjectAsInsider(Object object){
		  if (!super.CanAddObjectAsInsider(object)) return false;
      return PlayerBase.Cast(object) != null;
    }


  void Spatial_Spawn(int bod, string fac, string loa, string GroupName, int lootable){
    PlayerBase playerInsider = PlayerBase.Cast(m_insiders.Get(0).GetObject());
    vector startpos = ValidPos();
    TVectorArray waypoints = {
      ValidPos()
    };
    string Formation = "RANDOM";
    eAIWaypointBehavior behaviour = typename.StringToEnum(eAIWaypointBehavior, "ALTERNATE");
    int mindistradius, maxdistradius, despawnradius;
    mindistradius = 0;
    maxdistradius = 1000;
    despawnradius = 1200;
    bool UnlimitedReload = false;
    playerInsider.SetLocationHunt(Spatial_HuntMode);
    dynPatrol = eAISpatialPatrol.CreateEx(startpos, waypoints, behaviour, loa, bod, m_Spatial_Groups.CleanupTimer + 500, m_Spatial_Groups.CleanupTimer - 500, eAIFaction.Create(fac), eAIFormation.Create(Formation), playerInsider, mindistradius, maxdistradius, despawnradius, 2.0, 3.0, lootable, UnlimitedReload);
    playerInsider.SetLocationHunt(10);
    if (dynPatrol) {
      dynPatrol.SetGroupName(GroupName);
      dynPatrol.SetSniperProneDistanceThreshold(0.0);
      dynPatrol.SetLocation();
    }
  } //Spatial_Spawn(player, SpawnCount, faction, loadout, groupname, lootable)

  vector ValidPos(){
    return ExpansionStatic.GetSurfacePosition(Spatial_SpawnPosition);
  }
}
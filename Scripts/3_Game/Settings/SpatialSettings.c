/*
19/7/2023
spatial settings
*/
class SpatialSettings
{
  //declares
  private const static string EXP_SPATIAL_FOLDER = "$profile:ExpansionMod\\AI\\Spatial\\";
  private const static string EXP_AI_SPATIAL_SETTINGS = EXP_SPATIAL_FOLDER + "SpatialSettings.json";
  const int SZ_IN_SAFEZONE = 0x0001;
  int m_cur = 0;
  int m_Spatial_Total = -1;
  bool m_Debug = false;
  bool Spatial_Version = true;
  bool m_Spatial_InVehicle, m_Spatial_IsBleeding, m_Spatial_IsRestrained, m_Spatial_IsUnconscious, m_Spatial_IsInSafeZone, m_Spatial_TPSafeZone, m_Spatial_InOwnTerritory;

  //refs
  ref Spatial_Groups m_Spatial_Groups;

  bool Init()
  {
    Load();
    if (m_Spatial_Total < 0)
    {
      Print("Spatial AI Config Error - Disabled");
      loggerPrint("No Spatial Groups valid.");
      loggerPrint("At least one valid group Required"); //Sadface box
      return false;
    } else {
      if (Spatial_Version)
      {
        return Spatial_Version;
      }
    }
    return false;
  } //init

  void PullRef(out Spatial_Groups Data)
  {
    if (!m_Spatial_Groups) Load();
    Data = m_Spatial_Groups;
  } //load ref to use location

  void Load()
  {
    if (!FileExist(EXP_AI_SPATIAL_SETTINGS))
    {
      if (!FileExist(EXP_SPATIAL_FOLDER))
        MakeDirectory(EXP_SPATIAL_FOLDER);
      loggerPrint("WARNING: Couldn't find config file !");
      loggerPrint("Spatial config will be located in: " + EXP_SPATIAL_FOLDER);
      DefaultSpatialSettings(m_Spatial_Groups);
      JsonFileLoader < Spatial_Groups > .JsonSaveFile(EXP_AI_SPATIAL_SETTINGS, m_Spatial_Groups);
    } else {
      m_Spatial_Groups = new Spatial_Groups();
      JsonFileLoader < Spatial_Groups > .JsonLoadFile(EXP_AI_SPATIAL_SETTINGS, m_Spatial_Groups);
    }
    if (m_Spatial_Groups.Version != 18) { // dont like this. change it.
      loggerPrint("Settings File Out of date. Please delete and restart server.");
      Spatial_Version = false;
      return;
    }
    if (m_Spatial_Groups.MaxAI < 1)
    {
      loggerPrint("MaxAI set under 1, disabling Spatial AI.");
      Spatial_Version = false;
      return;
    }

    if (m_Spatial_Groups.Spatial_MinTimer < 300000) { // global minimum time of 5 mins, 1m for debug 
      if (m_Spatial_Groups.Spatial_MinTimer < -59999)
      {
        m_Debug = 1;
        m_Spatial_Groups.Spatial_MinTimer = Math.AbsFloat(m_Spatial_Groups.Spatial_MinTimer);
      } else {
      loggerPrint("Timer Set either under 5m standard or under 1m debug - set to 5m");
      m_Spatial_Groups.Spatial_MinTimer = 300000;
      }
    }
    if (m_Spatial_Groups.Spatial_MaxTimer < m_Spatial_Groups.Spatial_MinTimer)
    {
      loggerPrint("Max Timer set lower than min timer, setting to the same.");
      m_Spatial_Groups.Spatial_MaxTimer = m_Spatial_Groups.Spatial_MinTimer;
    }
    if (m_Spatial_Groups.MinDistance < 120)
    {
      loggerPrint("Minimum Distance too low. setting to 120m");
      m_Spatial_Groups.MinDistance = 120;
    }
    if (m_Spatial_Groups.MaxDistance < m_Spatial_Groups.MinDistance)
    {
      loggerPrint("Max distance under min distance. setting +20m");
      m_Spatial_Groups.MaxDistance = m_Spatial_Groups.MinDistance + 20;
    }
    if (m_Spatial_Groups.HuntMode < 0 || m_Spatial_Groups.HuntMode > 8)
    {
      loggerPrint("HuntMode setting wrong. setting to default.");
      m_Spatial_Groups.HuntMode = 1;
    }
    if (m_Spatial_Groups.Points_Enabled == 0) {} else if (m_Spatial_Groups.Points_Enabled == 1 || m_Spatial_Groups.Points_Enabled == 2 )
    {
      foreach(Spatial_Point point: m_Spatial_Groups.Point)
      {
        if (point.Spatial_Radius < 0)
        {
          loggerPrint("Radius on group incorrect, setting to 100m");
          point.Spatial_Radius = 100;
        }
        if (!FileExist("$profile:ExpansionMod\\Loadouts\\" + point.Spatial_ZoneLoadout))
        {
          loggerPrint("Loadout Not Found: " + point.Spatial_ZoneLoadout);
          point.Spatial_ZoneLoadout = "HumanLoadout.json";
        }
        if (point.Spatial_Safe != 1 && point.Spatial_Safe != 0)
        {
          loggerPrint("Zone safe value incorrect. setting to safe");
          point.Spatial_Safe = 1;
        }
        eAIFaction faction = eAIFaction.Create(point.Spatial_Faction);
        if (!faction)
        {
          loggerPrint("Faction not correct: " + point.Spatial_Faction);
          point.Spatial_Faction = "Raiders";
        }
        
        if (point.Spatial_HuntMode < 0 || point.Spatial_HuntMode > 8)
        {
          loggerPrint("Point huntmode set incorrectly. setting to 3.");
          point.Spatial_HuntMode = 3;
        }

        if (!point.Spatial_Name || point.Spatial_Name == "")
        {
          loggerPrint("Point Name set incorrectly. setting to Survivor");
          point.Spatial_Name = "Survivor";
        }
      }
    } else {
      loggerPrint("error, disabling points");
      m_Spatial_Groups.Points_Enabled = 0;
    }

    if (m_Spatial_Groups.Locations_Enabled != 0 && m_Spatial_Groups.Locations_Enabled != 1 && m_Spatial_Groups.Locations_Enabled != 2)
    {
      loggerPrint("Locations_Enabled error, setting to false");
      m_Spatial_Groups.Locations_Enabled = 0;
    }

    if (m_Spatial_Groups.EngageTimer < 300000)
    {
      loggerPrint("Minimum engagement too low. setting to 5m");
      m_Spatial_Groups.EngageTimer = 300000;
    } // global minimum time of 5 mins

    if (m_Spatial_Groups.CleanupTimer < m_Spatial_Groups.EngageTimer)
    {
      loggerPrint("Cleanup timer under engage timer. setting +1m");
      m_Spatial_Groups.CleanupTimer = m_Spatial_Groups.EngageTimer + 60000;
    }

    if (m_Spatial_Groups.MessageType < 0 && m_Spatial_Groups.MessageType > 5)
    {
      loggerPrint("Message type error. disabling.");
      m_Spatial_Groups.MessageType = 0;
    }

    if (!m_Spatial_Groups.MessageTitle)
    {
      loggerPrint("Notification title error. setting default.");
      m_Spatial_Groups.MessageTitle = "Spatial AI";
    } else if (m_Spatial_Groups.MessageTitle == " ")
    {
      loggerPrint("Notification title error. setting default.");
      m_Spatial_Groups.MessageTitle = "Spatial AI";
    }
    if (!m_Spatial_Groups.MessageText)
    {
      loggerPrint("Message text error. setting default.");
      m_Spatial_Groups.MessageText = "AI Spotted in the Area. Be Careful.";
    } else if (m_Spatial_Groups.MessageText == " ")
    {
      loggerPrint("Message text error. setting default.");
      m_Spatial_Groups.MessageText = "AI Spotted in the Area. Be Careful.";
    }
    if (m_Spatial_Groups.Spatial_InVehicle != true && m_Spatial_Groups.Spatial_InVehicle != false)
    {
      loggerPrint("ignore vehicle check error - default off");
      m_Spatial_Groups.Spatial_InVehicle = false;
    }
    m_Spatial_InVehicle = m_Spatial_Groups.Spatial_InVehicle;
    if (m_Spatial_Groups.Spatial_IsBleeding != true && m_Spatial_Groups.Spatial_IsBleeding != false)
    {
      loggerPrint("ignore bleeding check error - default off");
      m_Spatial_Groups.Spatial_IsBleeding = false;
    }
    m_Spatial_IsBleeding = m_Spatial_Groups.Spatial_IsBleeding;
    if (m_Spatial_Groups.Spatial_IsRestrained != true && m_Spatial_Groups.Spatial_IsRestrained != false)
    {
      loggerPrint("ignore restrained check error - default off");
      m_Spatial_Groups.Spatial_IsRestrained = false;
    }
    m_Spatial_IsRestrained = m_Spatial_Groups.Spatial_IsRestrained;
    if (m_Spatial_Groups.Spatial_IsUnconscious != true && m_Spatial_Groups.Spatial_IsUnconscious != false)
    {
      loggerPrint("ignore Unconscious check error - default off");
      m_Spatial_Groups.Spatial_IsUnconscious = false;
    }
    m_Spatial_IsUnconscious = m_Spatial_Groups.Spatial_IsUnconscious;
    if (m_Spatial_Groups.Spatial_IsInSafeZone != true && m_Spatial_Groups.Spatial_IsInSafeZone != false)
    {
      loggerPrint("ignore expansion SafeZone check error - default off");
      m_Spatial_Groups.Spatial_IsInSafeZone = false;
    }
    m_Spatial_IsInSafeZone = m_Spatial_Groups.Spatial_IsInSafeZone;
    if (m_Spatial_Groups.Spatial_TPSafeZone != true && m_Spatial_Groups.Spatial_TPSafeZone != false)
    {
      loggerPrint("ignore traderplus SafeZone check error - default off");
      m_Spatial_Groups.Spatial_TPSafeZone = false;
    }
    m_Spatial_TPSafeZone = m_Spatial_Groups.Spatial_TPSafeZone;
    if (m_Spatial_Groups.Spatial_InOwnTerritory != true && m_Spatial_Groups.Spatial_InOwnTerritory != false)
    {
      loggerPrint("ignore expansion own territory check error - default off");
      m_Spatial_Groups.Spatial_InOwnTerritory = false;
    }
    m_Spatial_InOwnTerritory = m_Spatial_Groups.Spatial_InOwnTerritory;
    m_Spatial_Total = 0;
    foreach(Spatial_Group group: m_Spatial_Groups.Group)
    {
      if (group.Spatial_MinCount < 0)
      {
        group.Spatial_MinCount = 0;
        loggerPrint("Group Spatial_MinCount error, setting to 0");
        continue;
      }

      if (!group.Spatial_Name || group.Spatial_Name == "")
      {
          loggerPrint("group Name set incorrectly. setting to Survivor");
          group.Spatial_Name = "Survivor";
      }

      if (group.Spatial_MinCount > group.Spatial_MaxCount)
      {
        loggerPrint("Minimum cant be more than maximum: " + group.Spatial_MinCount);
        continue;
      }
      if (group.Spatial_MaxCount < 1)
      {
        loggerPrint("Maximum count cant be less than 0: " + group.Spatial_MaxCount);
        continue;
      }
      if (!FileExist("$profile:ExpansionMod\\Loadouts\\" + group.Spatial_Loadout))
      {
        loggerPrint("Loadout Not Found: " + group.Spatial_Loadout);
        continue;
      }
      eAIFaction faction2 = eAIFaction.Create(group.Spatial_Faction);
      if (!faction2)
      {
        loggerPrint("Faction not correct: " + group.Spatial_Faction);
        group.Spatial_Faction = "Raiders";
      }
      m_Spatial_Total += 1;
    }
  } //load from file/data checks

  void DefaultSpatialSettings(out Spatial_Groups Data)
  {
    Data = new Spatial_Groups();
    Data.Group.Insert(new Spatial_Group(2, 2, 200, "NBCLoadout.json", "Guards", "Guard", 1, 0.5, 0.25, 0.66, false));
    Data.Group.Insert(new Spatial_Group(1, 3, 300, "HumanLoadout.json", "Shamans", "Shaman", 1, 0.5, 0.25, 0.66, true));
    Data.Group.Insert(new Spatial_Group(2, 3, 350, "EastLoadout.json", "Passive", "Passive", 1, 0.5, 0.25, 0.66, true));

    Data.Point.Insert(new Spatial_Point("East", 0, 50, "EastLoadout.json", 0, 4, 6, "East", 1, 0.5, 0.25, 0.66, false, "0.0 0.0 0.0"));
    Data.Point.Insert(new Spatial_Point("West", 0, 100, "WestLoadout.json", 0, 5, 1, "West", 1, 0.5, 0.25, 0.66, false, "0.0 0.0 0.0"));
    Data.Point.Insert(new Spatial_Point("Civilian", 1, 150, "HumanLoadout.json", 0, 0, 0, "Civilian", 1, 0.5, 0.25, 0.66, false, "0.0 0.0 0.0"));

    Data.Location.Insert(new Spatial_Location("Name", 50, "NBCLoadout.json", 0, 4, 6, "East", 1, 0.5, 0.25, 0.66, 60000, false, "0.0 0.0 0.0", "0.0 0.0 0.0"));
    Data.Location.Insert(new Spatial_Location("Name", 50, "NBCLoadout.json", 0, 4, 6, "East", 1, 0.5, 0.25, 0.66, 60000, false, "0.0 0.0 0.0", "0.0 0.0 0.0"));
  } //generate default array data

  void loggerPrint(string msg)
  {
    if (GetExpansionSettings().GetLog().AIGeneral)
      GetExpansionSettings().GetLog().PrintLog("[Spatial Settings] " + msg);
  } //expansion logging (Spatial AI prefex)

  //returns
  int Spatial_Total()
  {
    return m_Spatial_Total;
  }

  bool Spatial_Debug()
  {
    return m_Debug;
  }

  bool Spatial_InVehicle()
  {
    return m_Spatial_InVehicle;
  }

  bool Spatial_IsBleeding()
  {
    return m_Spatial_IsBleeding;
  }

  bool Spatial_IsRestrained()
  {
    return m_Spatial_IsRestrained;
  }

  bool Spatial_IsUnconscious()
  {
    return m_Spatial_IsUnconscious;
  }

  bool Spatial_IsInSafeZone()
  {
    return m_Spatial_IsInSafeZone;
  }

  bool Spatial_TPSafeZone()
  {
    return m_Spatial_TPSafeZone;
  }

  bool Spatial_InOwnTerritory()
  {
    return m_Spatial_InOwnTerritory;
  }

  TStringArray Spatial_WhiteList()
  {
    if (!m_Spatial_Groups) Load();
    return m_Spatial_Groups.LootWhitelist;
  }
};

//json data
class Spatial_Groups
{
  int Version = 18;
  int Spatial_MinTimer = 1200000;
  int Spatial_MaxTimer = 1200000;
  int MinDistance = 140;
  int MaxDistance = 220;
  int HuntMode = 1;
  int Points_Enabled = 0;
  int Locations_Enabled = 0;
  int EngageTimer = 300000;
  int CleanupTimer = 360000;
  int PlayerChecks = 0;
  int MaxAI = 20;
  int GroupDifficulty = 1;
  int MinimumPlayerDistance = 0;
  int MaxSoloPlayers = 6;
  int MinimumAge = 0;
  int MessageType = 1;
  string MessageTitle = "Spatial AI";
  string MessageText = "AI Spotted in the Area. Be Careful.";
  ref TStringArray LootWhitelist = {};
  bool Spatial_InVehicle = false;
  bool Spatial_IsBleeding = false;
  bool Spatial_IsRestrained = false;
  bool Spatial_IsUnconscious = false;
  bool Spatial_IsInSafeZone = false;
  bool Spatial_TPSafeZone = false;
  bool Spatial_InOwnTerritory = false;
  ref array < ref Spatial_Group > Group;
  ref array < ref Spatial_Point > Point;
  ref array < ref Spatial_Location > Location;

  void Spatial_Groups()
  {
    Group = new array < ref Spatial_Group > ;
    Point = new array < ref Spatial_Point > ;
    Location = new array < ref Spatial_Location > ;
  }
};

class Spatial_Group
{
  int  Spatial_MinCount, Spatial_MaxCount;
  float  Spatial_Weight;
  string  Spatial_Loadout, Spatial_Faction, Spatial_Name;
  int  Spatial_Lootable;
  float  Spatial_Chance, Spatial_MinAccuracy, Spatial_MaxAccuracy;
  bool Spatial_UnlimitedReload;

  void Spatial_Group(int a, int b, float c, string d, string e, string f, int g, float h, float i, float j, bool k)
  {
    Spatial_MinCount = a;
    Spatial_MaxCount = b;
    Spatial_Weight = c;
    Spatial_Loadout = d;
    Spatial_Faction = e;
    Spatial_Name = f;
    Spatial_Lootable = g;
    Spatial_Chance = h;
    Spatial_MinAccuracy = i;
    Spatial_MaxAccuracy = j;
    Spatial_UnlimitedReload = k;
  }
};

class Spatial_Point
{
  string Spatial_Name;
  bool Spatial_Safe;
  float Spatial_Radius;
  string Spatial_ZoneLoadout;
  int Spatial_MinCount, Spatial_MaxCount;
  int Spatial_HuntMode;
  string Spatial_Faction;
  int Spatial_Lootable;
  float Spatial_Chance, Spatial_MinAccuracy, Spatial_MaxAccuracy;
  bool Spatial_UnlimitedReload;
  vector Spatial_Position;

  void Spatial_Point(string i, bool a, float b, string c, int d, int e, int h, string f, int j, float k, float m, float n, bool l, vector g)
  {
    Spatial_Name = i;
    Spatial_Safe = a;
    Spatial_Radius = b;
    Spatial_ZoneLoadout = c;
    Spatial_MinCount = d;
    Spatial_MaxCount = e;
    Spatial_HuntMode = h;
    Spatial_Faction = f;
    Spatial_Lootable = j;
    Spatial_Chance = k;
    Spatial_MinAccuracy = m;
    Spatial_MaxAccuracy = n;
    Spatial_UnlimitedReload = l;
    Spatial_Position = g;
  }
};

class Spatial_Location
{
  string Spatial_Name;
  float  Spatial_TriggerRadius;
  string  Spatial_ZoneLoadout;
  int Spatial_MinCount, Spatial_MaxCount;
  int  Spatial_HuntMode;
  string  Spatial_Faction;
  int  Spatial_Lootable;
  float  Spatial_Chance, Spatial_MinAccuracy, Spatial_MaxAccuracy, Spatial_Timer;
  bool Spatial_UnlimitedReload;
  vector  Spatial_TriggerPosition, Spatial_SpawnPosition;

  void Spatial_Location(string i, float b, string c, int d, int e, int h, string f, int j, float k, float n, float o, float l, bool m, vector g, vector a )
  {
    Spatial_Name = i;
    Spatial_TriggerRadius = b;
    Spatial_ZoneLoadout = c;
    Spatial_MinCount = d;
    Spatial_MaxCount = e;
    Spatial_HuntMode = h;
    Spatial_Faction = f;
    Spatial_Lootable = j;
    Spatial_Chance = k;
    Spatial_MinAccuracy = n;
    Spatial_MaxAccuracy = o;
    Spatial_Timer = l;
    Spatial_UnlimitedReload = m;
    Spatial_TriggerPosition = g;
    Spatial_SpawnPosition = a;
  }
};


static ref SpatialSettings g_SpatialSettings;
static SpatialSettings GetSpatialSettings()
{
  if (g_SpatialSettings == NULL)
  {
    g_SpatialSettings = new SpatialSettings();
    g_SpatialSettings.Load();
  }
  return g_SpatialSettings;
}
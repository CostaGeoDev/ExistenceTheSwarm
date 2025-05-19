class SpawnLocation
{
    string LocationName;
    vector Position;

    void SpawnLocation(string name, vector pos)
    {
        LocationName = name;
        Position = pos;
    }
}

class TheSwarmConfig
{
    // Config file location in the server profile directory
    private const static string swarmModFolder = "$profile:\\TheSwarm\\";
    private const static string swarmConfigName = "TheSwarmConfig.json";

    // Static constant config version (used for versioning, not saved to JSON)
    static const string CONFIG_VERSION = "0.0.3";

    // Data fields stored in the configuration
    string ConfigVersion = ""; // Stores the current config version

    // Time in seconds after the server starts to spawn the horde - default is 10 minutes
    int spawnDelay = 600;

    // Time in seconds between each horde spawn - default is 30 minutes
    int spawnInterval = 1800;

    // Display messages when the horde spawns - default is true
    bool displayMessages = true; 

    // Time in seconds for the horde to despawn if no players are nearby - default is 10 minutes
    int despawnTime = 600;

    // Radius in meters to check for players before despawning the horde - default is 500 meters
    float despawnRadius = 500.0;

    // Define minimum and maximum amount of infected
    int minInfected = 10;
    int maxInfected = 50;

    // Minimum and maximum distance from the spawn location to the player
    float safeSpawnRadius = 50.0;    

    // Array of locations to spawn the horde
    autoptr array<ref SpawnLocation> spawnLocations = {
        new SpawnLocation("Bielawa", "1558.59 0.0 9718.75"),
        new SpawnLocation("Branzow Castle", "1048.24 0.0 11416.80"),
    };

    // Array of messages to display when the horde spawns
    autoptr array<string> spawnMessages = {
        "A horde of Infected have been seen",
        "The sound of Infected can be heard somewhere",
        "Infected have been spotted",
        "The Infected are on the move",
        "The Infected are coming",
        "The Infected are closing in",
        "The Infected are",
        "The Infected are approaching"
    };

    // Array defining possible infected types to spawn
    autoptr array<string> infectedTypes = {
        "ZmbM_CitizenASkinny_Brown",
        "ZmbM_priestPopSkinny",
        "ZmbM_HermitSkinny_Beige",
        "ZmbF_JoggerSkinny_Red",
        "ZmbF_BlueCollarFat_Green",
        "ZmbM_PatrolNormal_Summer",
        "ZmbM_CitizenBFat_Blue",
        "ZmbF_HikerSkinny_Grey",
        "ZmbF_JournalistNormal_White",
        "ZmbF_SkaterYoung_Striped",
        "ZmbM_Jacket_black",
        "ZmbM_PolicemanSpecForce",
        "ZmbM_Jacket_stripes",
        "ZmbM_HikerSkinny_Blue",
        "ZmbM_HikerSkinny_Yellow",
        "ZmbM_PolicemanFat",
        "ZmbM_JoggerSkinny_Blue",
        "ZmbM_VillagerOld_White",
        "ZmbM_SkaterYoung_Brown",
        "ZmbM_MechanicSkinny_Green",
        "ZmbM_DoctorFat",
        "ZmbM_PatientSkinny",
        "ZmbM_ClerkFat_Brown",
        "ZmbM_ClerkFat_White",
        "ZmbM_Jacket_magenta"
    };

    // Loads the configuration file, or creates a new one if it doesn't exist
    void Load()
    {
        // Check if the config file exists
        if (FileExist(swarmModFolder + swarmConfigName))
        {
            // Load the existing config file
            JsonFileLoader<TheSwarmConfig>.JsonLoadFile(swarmModFolder + swarmConfigName, this);

            // If the version doesn't match, backup the old version
            if (ConfigVersion != CONFIG_VERSION)
            {
                JsonFileLoader<TheSwarmConfig>.JsonSaveFile(swarmModFolder + swarmConfigName + "_old", this);
            }
            else
            {
                // If the config version matches, no further action is needed
                return;
            }
        }

        // If the config file doesn't exist, set default values
        ConfigVersion = CONFIG_VERSION;

        // Save the default config
        Save();
    }

    // Saves the configuration file
    void Save()
    {
        // If the folder doesn't exist, create it
        if (!FileExist(swarmModFolder))
        {
            MakeDirectory(swarmModFolder);
        }

        // Save the configuration file
        JsonFileLoader<TheSwarmConfig>.JsonSaveFile(swarmModFolder + swarmConfigName, this);
    }
}

// Reference to the global configuration object
ref TheSwarmConfig m_TheSwarmConfig;

// Function to access the configuration object
static TheSwarmConfig GetTheSwarmConfig()
{
    // Initialize the config only if it doesn't already exist and is running on a dedicated server
    if (!m_TheSwarmConfig && GetGame().IsDedicatedServer())
    {
        Print("[TheSwarmConfig] Initializing configuration...");
        m_TheSwarmConfig = new TheSwarmConfig;
        m_TheSwarmConfig.Load();
    }

    return m_TheSwarmConfig;
}

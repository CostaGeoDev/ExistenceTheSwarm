class TheSwarm
{
    private ref TheSwarmConfig config;
    private ref array<EntityAI> currentHorde = new array<EntityAI>(); // Track the currently spawned horde

    void TheSwarm()
    {
        config = GetTheSwarmConfig();
    }

    private void GetMapBounds(out float minX, out float maxX, out float minZ, out float maxZ)
    {
        string world = config.WorldName;
        if (world == "Chernarus")
        {
            minX = 0; maxX = 15360;
            minZ = 0; maxZ = 15360;
        }
        else if (world == "Livonia")
        {
            minX = 0; maxX = 12800;
            minZ = 0; maxZ = 12800;
        }
        else if (world == "Namalsk")
        {
            minX = 0; maxX = 8192;
            minZ = 0; maxZ = 8192;
        }
        else if (world == "Deerisle" || world == "DeerIsle")
        {
            minX = 0; maxX = 16384;
            minZ = 0; maxZ = 16384;
        }
        else if (world == "Esseker")
        {
            minX = 0; maxX = 10240;
            minZ = 0; maxZ = 10240;
        }
        else if (world == "Banov")
        {
            minX = 0; maxX = 10240;
            minZ = 0; maxZ = 10240;
        }
        else if (world == "Iztek")
        {
            minX = 0; maxX = 10240;
            minZ = 0; maxZ = 10240;
        }
        else if (world == "Rostow")
        {
            minX = 0; maxX = 12288;
            minZ = 0; maxZ = 12288;
        }
        else if (world == "Takistan")
        {
            minX = 0; maxX = 12800;
            minZ = 0; maxZ = 12800;
        }
        else
        {
            // Default fallback (Livonia size)
            minX = 0; maxX = 12800;
            minZ = 0; maxZ = 12800;
        }
    }

    void SpawnHorde()
    {
        if (config.spawnLocations.Count() == 0 && !config.randomSpawnLocation)
        {
            Print("[TheSwarm] No spawn locations defined and random spawn disabled. Aborting horde spawn.");
            return;
        }

        DespawnHorde();

        float SAFE_RADIUS = config.safeSpawnRadius;
        SpawnLocation chosenLocation;
        vector chosenPosition;
        string chosenName;
        bool foundSafeLocation = false;

        int totalOptions = config.spawnLocations.Count();
        if (config.randomSpawnLocation) totalOptions += 1; // Add one for the random option

        for (int attempt = 0; attempt < totalOptions; attempt++)
        {
            bool pickRandom = config.randomSpawnLocation && Math.RandomInt(0, totalOptions) == 0;
            if (pickRandom)
            {
                float minX, maxX, minZ, maxZ;
                GetMapBounds(minX, maxX, minZ, maxZ);
                float x, z, y;
                bool validLand = false;
                int maxLandTries = 10;
                for (int landTry = 0; landTry < maxLandTries; landTry++)
                {
                    x = Math.RandomFloat(minX, maxX);
                    z = Math.RandomFloat(minZ, maxZ);
                    if (!GetGame().SurfaceIsSea(x, z))
                    {
                        validLand = true;
                        break;
                    }
                }
                if (!validLand)
                    continue; // Try another random or preset location

                y = GetGame().SurfaceY(x, z);
                chosenPosition = Vector(x, y, z);
                chosenName = "Random Location";
            }
            else
            {
                chosenLocation = config.spawnLocations.GetRandomElement();
                chosenPosition = chosenLocation.Position;
                chosenName = chosenLocation.LocationName;
            }

            // Check for players nearby
            bool playerTooClose = false;
            array<Man> players = new array<Man>();
            GetGame().GetPlayers(players);

            foreach (Man player : players)
            {
                if (player && vector.Distance(player.GetPosition(), chosenPosition) <= SAFE_RADIUS)
                {
                    playerTooClose = true;
                    break;
                }
            }

            if (!playerTooClose)
            {
                foundSafeLocation = true;
                break;
            }
        }

        if (!foundSafeLocation)
        {
            Print("[TheSwarm] No safe spawn location found (all too close to players). Aborting horde spawn.");
            return;
        }

        int infectedAmount = Math.RandomInt(config.minInfected, config.maxInfected + 1);

        Print("[TheSwarm] Spawning horde of " + infectedAmount + " infected at " + chosenName + " (" + chosenPosition.ToString() + ").");

        if (config.displayMessages)
        {
            string randomMessage = config.spawnMessages.GetRandomElement();
            GetGame().ChatPlayer(randomMessage + " near " + chosenName);
        }

        for (int i = 0; i < infectedAmount; i++)
        {
            string infectedType = config.infectedTypes.GetRandomElement();
            vector spawnPos = chosenPosition + Vector(Math.RandomFloat(-5, 5), 0, Math.RandomFloat(-5, 5));
            EntityAI infected = GetGame().CreateObject(infectedType, spawnPos, false, true, true);
            if (infected)
            {
                currentHorde.Insert(infected);
                Print("[TheSwarm] Spawned infected " + infectedType + " at " + spawnPos.ToString());
            }
            else
            {
                Print("[TheSwarm] Failed to spawn infected " + infectedType + " at " + spawnPos.ToString());
            }
        }

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(CheckAndDespawnHorde, 5000, true); // Check every 5 seconds
    }

    // Despawns the current horde if it exists
    void DespawnHorde()
    {
        if (currentHorde.Count() > 0)
        {
            Print("[TheSwarm] Despawning the previous horde...");

            // Loop through all the infected and despawn them
            foreach (EntityAI infected : currentHorde)
            {
                if (infected)
                {
                    GetGame().ObjectDelete(infected); // Delete the infected object
                    Print("[TheSwarm] Deleted infected at " + infected.GetPosition().ToString());
                }
            }

            // Clear the list of current horde members
            currentHorde.Clear();
        }

        // Stop proximity checks if no horde exists
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(CheckAndDespawnHorde);
    }

    // Add a flag to track if the initial log message has been displayed
    private bool hasLoggedWaitingMessage = false;
    private bool hasLoggedActiveMessage = false;

    // Check if there are players nearby and despawn if no players are found after despawnTime
    void CheckAndDespawnHorde()
    {
        if (currentHorde.Count() == 0) return; // No horde to check

        bool playersNearby = false;
        array<Man> players = new array<Man>();
        GetGame().GetPlayers(players);

        // Check each player against the first infected's position
        foreach (Man player : players)
        {
            if (player && vector.Distance(player.GetPosition(), currentHorde[0].GetPosition()) <= config.despawnRadius)
            {
                playersNearby = true;
                break;
            }
        }

        if (!playersNearby)
        {
            // Only log the "waiting" message if it's the first time we're checking and no players are nearby
            if (!hasLoggedWaitingMessage)
            {
                Print("[TheSwarm] No players near the horde. Waiting for " + config.despawnTime + " seconds before despawning...");
                hasLoggedWaitingMessage = true;  // Set the flag so it doesn't log again
            }

            // Schedule the despawn only if no players are within range and the time hasn't run out
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(DelayedDespawnCheck, config.despawnTime * 1000, false);
        }
        else
        {
            // Log only once that players are nearby
            if (!hasLoggedActiveMessage)
            {
                Print("[TheSwarm] Players detected near the horde. Keeping horde active.");
                hasLoggedActiveMessage = true;  // Set the flag so it doesn't log again
            }

            // Stop the periodic checks if players are nearby
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(CheckAndDespawnHorde);
        }
    }

    // Delayed despawn check after the despawnTime expires
    void DelayedDespawnCheck()
    {
        if (currentHorde.Count() == 0) return; // No horde to check again

        // Check for players once more before despawning
        bool playersNearby = false;
        array<Man> players = new array<Man>();
        GetGame().GetPlayers(players);

        foreach (Man player : players)
        {
            if (player && vector.Distance(player.GetPosition(), currentHorde[0].GetPosition()) <= config.despawnRadius)
            {
                playersNearby = true;
                break;
            }
        }

        if (!playersNearby)
        {
            Print("[TheSwarm] No players found after despawn time. Despawning the horde...");
            DespawnHorde(); // Proceed to despawn the horde after the timer
        }
        else
        {
            Print("[TheSwarm] Players detected near the horde after the timer. Keeping horde active.");
        }

        // Reset flags to allow logging again if a new horde is spawned
        hasLoggedWaitingMessage = false;
        hasLoggedActiveMessage = false;
    }
}

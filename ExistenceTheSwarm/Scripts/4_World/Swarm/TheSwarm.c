class TheSwarm
{
    private ref TheSwarmConfig config;
    private ref array<EntityAI> currentHorde = new array<EntityAI>(); // Track the currently spawned horde

    void TheSwarm()
    {
        config = GetTheSwarmConfig();
    }

    void SpawnHorde()
    {
        if (config.spawnLocations.Count() == 0)
        {
            Print("[TheSwarm] No spawn locations defined. Aborting horde spawn.");
            return;
        }

        // Ensure the previous horde is despawned before spawning a new one
        DespawnHorde();

        // Try to find a spawn location not near any player
        float SAFE_RADIUS = config.safeSpawnRadius;
        SpawnLocation randomLocation;
        bool foundSafeLocation = false;

        for (int attempt = 0; attempt < config.spawnLocations.Count(); attempt++)
        {
            SpawnLocation candidate = config.spawnLocations.GetRandomElement();
            vector pos = candidate.Position;
            bool playerTooClose = false;

            array<Man> players = new array<Man>();
            GetGame().GetPlayers(players);

            foreach (Man player : players)
            {
                if (player && vector.Distance(player.GetPosition(), pos) <= SAFE_RADIUS)
                {
                    playerTooClose = true;
                    break;
                }
            }

            if (!playerTooClose)
            {
                randomLocation = candidate;
                foundSafeLocation = true;
                break;
            }
        }

        if (!foundSafeLocation)
        {
            Print("[TheSwarm] No safe spawn location found (all too close to players). Aborting horde spawn.");
            return;
        }

        string locationName = randomLocation.LocationName;
        vector position = randomLocation.Position;

        // Randomize the number of infected to spawn
        int infectedAmount = Math.RandomInt(config.minInfected, config.maxInfected + 1); // +1 because the upper bound is exclusive

        // Log spawn information
        Print("[TheSwarm] Spawning horde of " + infectedAmount + " infected at " + locationName + " (" + position.ToString() + ").");

        if (config.displayMessages)
        {
            /// DisplaySpawnMessage(locationName, position);
            string randomMessage = config.spawnMessages.GetRandomElement();
            GetGame().ChatPlayer(randomMessage + " near " + locationName); // Broadcast the message to all players
        }

        // Spawn the infected
        for (int i = 0; i < infectedAmount; i++)
        {
            string infectedType = config.infectedTypes.GetRandomElement(); // Get a random infected type
            vector spawnPos = position + Vector(Math.RandomFloat(-5, 5), 0, Math.RandomFloat(-5, 5)); // Random offset around the location

            EntityAI infected = GetGame().CreateObject(infectedType, spawnPos, false, true, true); // Spawn the infected
            if (infected)
            {
                currentHorde.Insert(infected); // Add to the active horde list
                Print("[TheSwarm] Spawned infected " + infectedType + " at " + spawnPos.ToString());
            }
            else
            {
                Print("[TheSwarm] Failed to spawn infected " + infectedType + " at " + spawnPos.ToString());
            }
        }

        // Schedule the first check for despawn
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

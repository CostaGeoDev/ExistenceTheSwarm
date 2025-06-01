modded class MissionServer
{
    private ref TheSwarm swarm; // Persistent instance of TheSwarm

    override void OnInit()
    {   
        super.OnInit();
        
        // Ensure the configuration is loaded
        GetTheSwarmConfig();

        // Create an instance of TheSwarm class
        swarm = new TheSwarm();

        // Print that the configuration has been loaded for the swarm mod
        Print("[TheSwarmConfig] OnInit - Loaded config successfully.");

        // Set the delay for the first swarm to start
        int delay = GetTheSwarmConfig().spawnDelay;
        Print("[TheSwarmConfig] OnInit - Delaying first swarm by " + delay + " seconds.");

        // Schedule the first swarm to start after the delay
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SpawnSwarm, delay * 1000, false);

        // Schedule the swarm to spawn at regular intervals
        int interval = GetTheSwarmConfig().spawnInterval;
        Print("[TheSwarmConfig] OnInit - Scheduling swarm to spawn every " + interval + " seconds.");
    }

    // Function to spawn the swarm
    void SpawnSwarm()
    {
        if (swarm)
        {
            swarm.SpawnHorde(); // Ensure the previous horde is despawned before spawning a new one
        }

        // Reschedule the next swarm spawn after the configured interval
        int interval = GetTheSwarmConfig().spawnInterval;
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SpawnSwarm, interval * 1000, false);
    }
}

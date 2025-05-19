class CfgPatches
{
	class The_Swarm
	{
		units[]={};
		weapons[]={};
		requiredVersion=0.1;
		requiredAddons[]=
		{
			"DZ_Scripts",
			"DZ_Data"
		};
	};
};
class CfgMods
{
	class The_Swarm
	{
		dir="ExistenceTheSwarm";
		picture="";
		action="";
		hideName=1;
		hidePicture=1;
		name="The Swarm";
		credits="CostaGeo";
		author="";
		authorID="0";
		version="1.0";
		extra=0;
		type="mod";
		inputs="";
		dependencies[]=
		{
			"Game",
			"World",
			"Mission"
		};
		class defs
		{
			class gameScriptModule
			{
				value="";
				files[]=
				{
					"ExistenceTheSwarm/Scripts/3_Game"
				};
			};
			class worldScriptModule
			{
				value="";
				files[]=
				{
					"ExistenceTheSwarm/Scripts/4_World"
				};
			};
			class missionScriptModule
			{
				value="";
				files[]=
				{
					"ExistenceTheSwarm/Scripts/5_Mission"
				};
			};
		};
	};
};

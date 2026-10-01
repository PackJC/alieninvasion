// Writes Alien Invasion's Central Economy files into $profile:Gebs/mpmissions/ on server start, next to
// gebsfish's. Each file's second line is "<!-- Version: x -->"; a file is only rewritten when it is missing or
// its version differs from VERSION_ALIENINVASION, so admin edits survive restarts until the next mod update.
//
// Server owners copy types/spawnabletypes/events into mpmissions/<mission>/alieninvasion/ and register them with
// the block in alieninvasion-cfgeconomycore.xml; cfgeventspawns and mapgroupproto entries are merged by hand.
class geb_AlienInvasionFiles
{
	protected const string DIRECTORY_PATH = "$profile:Gebs/mpmissions/";
	protected const string VERSION_PREFIX = "<!-- Version: ";
	protected const string LOG_PREFIX = "[AlienInvasion] ";

	void GenerateAll()
	{
		if ( !GetGame() || !GetGame().IsServer() )
			return;

		MakeDirectory("$profile:Gebs");
		MakeDirectory(DIRECTORY_PATH);

		WriteTypes();
		WriteSpawnableTypes();
		WriteEvents();
		WriteEventSpawns();
		WriteMapGroupProto();
		WriteEconomyCore();
	}

	// ------------------------------------------------------------------------------------------------------------
	// Files
	// ------------------------------------------------------------------------------------------------------------

	protected void WriteTypes()
	{
		string name = "alieninvasion-types.xml";
		FileHandle file = BeginFile(name, "Register with alieninvasion-cfgeconomycore.xml, or merge into db/types.xml.");
		if ( !file )
			return;

		FPrintln(file, "<types>");
		FPrintln(file, "    <!-- Crash site (StaticAlienCrash event) and the aliens its script spawns -->");
		WriteType(file, "geb_Aliencrash", 0, 2100, 0, false, "");
		WriteType(file, "geb_GreenAlien", 0, 1800, 1, false, "");
		FPrintln(file, "    <!-- Gear (the crash site itself drops the rifle and cartridges) -->");
		WriteType(file, "geb_PlasmaRifle", 0, 1800, 1, false, "weapons");
		WriteType(file, "geb_PlasmaCartridge", 0, 1800, 1, false, "weapons");
		WriteType(file, "geb_FoilHat", 0, 1800, 1, false, "clothes");
		FPrintln(file, "    <!-- From skinning aliens -->");
		WriteType(file, "geb_GreenAlienMeat", 0, 14400, 0, true, "food");
		WriteType(file, "geb_GreenAlienSkin", 0, 28800, 0, true, "tools");
		FPrintln(file, "</types>");

		EndFile(file, name);
	}

	protected void WriteSpawnableTypes()
	{
		string name = "alieninvasion-spawnabletypes.xml";
		FileHandle file = BeginFile(name, "Register with alieninvasion-cfgeconomycore.xml, or merge into cfgspawnabletypes.xml.");
		if ( !file )
			return;

		FPrintln(file, "<spawnabletypes>");
		FPrintln(file, "    <type name=\"geb_PlasmaRifle\">");
		FPrintln(file, "        <attachments chance=\"0.50\">");
		FPrintln(file, "            <item name=\"geb_PlasmaCartridge\" chance=\"1.00\" />");
		FPrintln(file, "        </attachments>");
		FPrintln(file, "    </type>");
		FPrintln(file, "    <!-- Aliens sometimes carry a spare cell. Full guide: https://packjc.github.io/alieninvasion/ -->");
		FPrintln(file, "    <type name=\"geb_GreenAlien\">");
		FPrintln(file, "        <cargo chance=\"0.10\">");
		FPrintln(file, "            <item name=\"geb_PlasmaCartridge\" chance=\"1.00\" />");
		FPrintln(file, "        </cargo>");
		FPrintln(file, "    </type>");
		FPrintln(file, "</spawnabletypes>");

		EndFile(file, name);
	}

	protected void WriteEvents()
	{
		string name = "alieninvasion-events.xml";
		FileHandle file = BeginFile(name, "Register with alieninvasion-cfgeconomycore.xml, or merge into db/events.xml.");
		if ( !file )
			return;

		FPrintln(file, "<events>");
		FPrintln(file, "    <!-- Works like vanilla StaticHeliCrash. No InfectedAlien secondary event: each wreck spawns its own aliens, so a secondary would only duplicate them. -->");
		FPrintln(file, "    <event name=\"StaticAlienCrash\">");
		FPrintln(file, "        <nominal>3</nominal>");
		FPrintln(file, "        <min>0</min>");
		FPrintln(file, "        <max>0</max>");
		FPrintln(file, "        <lifetime>2100</lifetime>");
		FPrintln(file, "        <restock>0</restock>");
		FPrintln(file, "        <saferadius>1000</saferadius>");
		FPrintln(file, "        <distanceradius>1000</distanceradius>");
		FPrintln(file, "        <cleanupradius>1000</cleanupradius>");
		FPrintln(file, "        <flags deletable=\"1\" init_random=\"0\" remove_damaged=\"0\"/>");
		FPrintln(file, "        <position>fixed</position>");
		FPrintln(file, "        <limit>child</limit>");
		FPrintln(file, "        <active>1</active>");
		FPrintln(file, "        <children>");
		FPrintln(file, "            <child lootmax=\"15\" lootmin=\"10\" max=\"3\" min=\"1\" type=\"geb_Aliencrash\"/>");
		FPrintln(file, "        </children>");
		FPrintln(file, "    </event>");
		FPrintln(file, "</events>");

		EndFile(file, name);
	}

	// Crash positions are per map: the hand-picked Chernarus list, otherwise the mission's own heli crash spots.
	protected void WriteEventSpawns()
	{
		string name = "alieninvasion-cfgeventspawns.xml";
		FileHandle file = BeginFile(name, "Merge the StaticAlienCrash <event> into cfgeventspawns.xml (inside <eventposdef>).");
		if ( !file )
			return;

		string world = GetGame().GetWorldName();
		world.ToLower();

		FPrintln(file, "<eventposdef>");
		FPrintln(file, "    <event name=\"StaticAlienCrash\">");
		if ( world == "chernarusplus" )
		{
			FPrintln(file, "        <zone smin=\"1\" smax=\"3\" dmin=\"3\" dmax=\"5\" r=\"45\" />");
			TStringArray spots = GetChernarusCrashSpots();
			foreach (string spot : spots)
			{
				TStringArray xza = new TStringArray;
				spot.Split(" ", xza);
				FPrintln(file, "        <pos x=\"" + xza[0] + "\" z=\"" + xza[1] + "\" a=\"" + xza[2] + "\" />");
			}
		}
		else if ( !CopyHeliCrashSpots(file) )
		{
			FPrintln(file, "        <!-- No crash spots known for " + world + ": add <pos x=\"\" z=\"\" a=\"\" /> lines here -->");
		}
		FPrintln(file, "    </event>");
		FPrintln(file, "</eventposdef>");

		EndFile(file, name);
	}

	protected void WriteMapGroupProto()
	{
		string name = "alieninvasion-mapgroupproto.xml";
		FileHandle file = BeginFile(name, "Merge the <group> into mapgroupproto.xml (inside <prototype>) so the wreck gets loot points.");
		if ( !file )
			return;

		FPrintln(file, "<prototype>");
		FPrintln(file, "    <!-- Usage Military gives the wreck heli-crash loot. For custom UFO loot, add <usage name=\"UFO\"/> to");
		FPrintln(file, "         cfglimitsdefinition.xml, change the usage below to UFO, and give items <usage name=\"UFO\"/> with deloot=\"1\". -->");
		FPrintln(file, "    <group name=\"geb_Aliencrash\" lootmax=\"15\">");
		FPrintln(file, "        <usage name=\"Military\" />");
		FPrintln(file, "        <container name=\"lootFloor\" lootmax=\"15\">");
		FPrintln(file, "            <category name=\"tools\" />");
		FPrintln(file, "            <category name=\"containers\" />");
		FPrintln(file, "            <category name=\"clothes\" />");
		FPrintln(file, "            <category name=\"weapons\" />");
		FPrintln(file, "            <tag name=\"floor\" />");
		FPrintln(file, "            <tag name=\"shelves\" />");
		TStringArray points = GetCrashLootPoints();
		foreach (string point : points)
		{
			// "pos|range|height|flags", flags optional
			TStringArray parts = new TStringArray;
			point.Split("|", parts);
			string flags = "";
			if ( parts.Count() > 3 && parts[3] != "" )
				flags = " flags=\"" + parts[3] + "\"";
			FPrintln(file, "            <point pos=\"" + parts[0] + "\" range=\"" + parts[1] + "\" height=\"" + parts[2] + "\"" + flags + " />");
		}
		FPrintln(file, "        </container>");
		FPrintln(file, "    </group>");
		FPrintln(file, "</prototype>");

		EndFile(file, name);
	}

	protected void WriteEconomyCore()
	{
		string name = "alieninvasion-cfgeconomycore.xml";
		FileHandle file = BeginFile(name, "Copy the three files listed below into mpmissions/<your mission>/alieninvasion/ and paste the <ce> block inside <economycore> in cfgeconomycore.xml.");
		if ( !file )
			return;

		FPrintln(file, "<economycore>");
		FPrintln(file, "    <ce folder=\"alieninvasion\">");
		FPrintln(file, "        <file name=\"alieninvasion-types.xml\" type=\"types\" />");
		FPrintln(file, "        <file name=\"alieninvasion-spawnabletypes.xml\" type=\"spawnabletypes\" />");
		FPrintln(file, "        <file name=\"alieninvasion-events.xml\" type=\"events\" />");
		FPrintln(file, "    </ce>");
		FPrintln(file, "</economycore>");

		EndFile(file, name);
	}

	// ------------------------------------------------------------------------------------------------------------
	// Helpers
	// ------------------------------------------------------------------------------------------------------------

	// Opens fileName for writing with the XML/version header. Returns an empty handle when the file is already
	// at the current version (nothing to do) or can't be created.
	protected FileHandle BeginFile(string fileName, string usage)
	{
		FileHandle file;
		string path = DIRECTORY_PATH + fileName;
		if ( IsCurrentVersion(path) )
			return file;

		file = OpenFile(path, FileMode.WRITE);
		if ( !file )
		{
			Print(LOG_PREFIX + "Could not create " + path);
			return file;
		}

		FPrintln(file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
		FPrintln(file, VERSION_PREFIX + VERSION_ALIENINVASION + " -->");
		FPrintln(file, "<!-- Alien Invasion: " + usage + " -->");
		return file;
	}

	protected void EndFile(FileHandle file, string fileName)
	{
		CloseFile(file);
		Print(LOG_PREFIX + fileName + " generated in " + DIRECTORY_PATH);
	}

	protected bool IsCurrentVersion(string path)
	{
		if ( !FileExist(path) )
			return false;

		FileHandle file = OpenFile(path, FileMode.READ);
		if ( !file )
			return false;

		string line;
		string found = "";
		for (int i = 0; i < 5 && FGets(file, line) > 0; i++)
		{
			int start = line.IndexOf(VERSION_PREFIX);
			if ( start == -1 )
				continue;

			string tail = line.Substring(start + VERSION_PREFIX.Length(), line.Length() - start - VERSION_PREFIX.Length());
			int end = tail.IndexOf("-->");
			if ( end != -1 )
				found = tail.Substring(0, end).Trim();
			break;
		}

		CloseFile(file);
		return found == VERSION_ALIENINVASION;
	}

	protected void WriteType(FileHandle file, string typeName, int nominal, int lifetime, int min, bool crafted, string category)
	{
		string craftedFlag = "0";
		if ( crafted )
			craftedFlag = "1";

		FPrintln(file, "    <type name=\"" + typeName + "\">");
		FPrintln(file, "        <nominal>" + nominal.ToString() + "</nominal>");
		FPrintln(file, "        <lifetime>" + lifetime.ToString() + "</lifetime>");
		FPrintln(file, "        <restock>0</restock>");
		FPrintln(file, "        <min>" + min.ToString() + "</min>");
		FPrintln(file, "        <quantmin>-1</quantmin>");
		FPrintln(file, "        <quantmax>-1</quantmax>");
		FPrintln(file, "        <cost>100</cost>");
		FPrintln(file, "        <flags count_in_cargo=\"0\" count_in_hoarder=\"0\" count_in_map=\"1\" count_in_player=\"0\" crafted=\"" + craftedFlag + "\" deloot=\"0\"/>");
		if ( category != "" )
			FPrintln(file, "        <category name=\"" + category + "\"/>");
		FPrintln(file, "    </type>");
	}

	// Copies the <zone>/<pos> lines of the mission's StaticHeliCrash event, so UFOs can crash on any map.
	protected bool CopyHeliCrashSpots(FileHandle file)
	{
		string path = "$mission:cfgeventspawns.xml";
		if ( !FileExist(path) )
			return false;

		FileHandle source = OpenFile(path, FileMode.READ);
		if ( !source )
			return false;

		bool inHeliEvent = false;
		int copied = 0;
		string line;
		while ( FGets(source, line) >= 0 )
		{
			if ( !inHeliEvent )
			{
				if ( line.IndexOf("<event name=\"StaticHeliCrash\"") != -1 && line.IndexOf("/>") == -1 )
					inHeliEvent = true;
				continue;
			}

			if ( line.IndexOf("</event>") != -1 )
				break;

			if ( line.IndexOf("<pos ") != -1 || line.IndexOf("<zone ") != -1 )
			{
				line = line.Trim();
				if ( copied == 0 )
					FPrintln(file, "        <!-- Borrowed from this mission's StaticHeliCrash spots; remove any you want kept heli-only -->");
				FPrintln(file, "        " + line);
				copied++;
			}
		}

		CloseFile(source);
		return copied > 0;
	}

	// Hand-picked Chernarus crash sites ("x z angle").
	protected TStringArray GetChernarusCrashSpots()
	{
		TStringArray spots = {
			"11844.371094 6020.828613 357.697601", "10772.458984 7141.783691 236.599503", "6357.467285 5393.447754 4.996732",
			"9537.379883 6346.113769 298.912109", "6507.537598 3160.664062 241.351349", "9395.842773 8118.120605 168.518341",
			"3826.167236 725.191772 72.453087", "11606.522461 4326.942871 10.140474", "9916.824219 3880.88208 32.136044",
			"9935.674805 5458.304199 329.921448", "11722.352539 7612.571777 163.276627", "7122.116699 5103.704101 292.347717",
			"8957.305664 5408.166504 305.672455", "1902.029053 2396.375976 305.231537", "5421.377442 1441.891357 153.467041",
			"7781.773926 997.603821 236.305603", "10454.764648 4566.578125 71.130417", "10513.841797 7460.008789 14.157501",
			"11075.945312 2103.969238 7.593108", "12284.845703 6532.125488 168.800293", "10770.838867 5033.983398 14.402428",
			"9799.932617 5986.262207 296.511719", "9493.401367 6859.635254 332.174927", "11390.188477 5130.928223 171.016739",
			"10392.649414 6202.329102 226.997894", "11457.69043 6532.165527 300.234802", "11288.463867 3901.264893 341.482605",
			"10066.341797 7019.535645 60.353058", "11164.566406 6245.58252 25.130795", "12028.506836 5253.910644 276.524628",
			"9593.34375 8950.186523 169.633057", "12300.630859 4292.416504 107.773361", "12352.316406 3855.271484 247.376831",
			"9671.808594 1155.7594 65.545807", "11581.37793 3195.328857 299.695923", "11776.484375 930.251892 70.248642",
			"9270.135742 7127.370117 45.754696", "3976.023682 6856.164063 117.374977", "2639.861328 6405.301758 189.473114",
			"6727.75 6937.537109 1.371592", "6486.924316 7557.751953 270.646088", "2441.762207 4908.613281 195.498657",
			"6836.494629 8600.134766 190.550873", "4286.803223 6212.870117 132.904129", "5397.504883 6689.651367 176.442398",
			"740.898437 5043.588867 70.0037", "7265.016113 7720.161133 68.681", "6995.759766 6220.584473 4.310974",
			"7938.515137 6592.683594 4.849762", "8678.618164 6389.382813 26.257509", "4607.828125 6965.80957 200.789352",
			"7298.620117 8475.169922 300.332794", "2734.593262 5628.562988 210.831863", "7512.816406 7044.394531 42.374527",
			"5499.094238 7905.272461 310.571228", "6220.04834 6929.438477 297.638458", "7796.510742 7870.70459 112.280243",
			"7900.789551 8257.776367 171.445618", "8546.589844 8372.402344 250.377197", "7776.442383 8989.429688 69.693436",
			"8356.177734 9151.981445 134.569748", "9198.206055 6041.199219 285.832367", "8241.619141 6025.999512 141.819946",
			"8490.319336 2832.061035 27.825129", "7188.131348 3713.572266 338.102417", "6369.329102 6150.516602 162.933716",
			"4725.089844 3215.219727 331.929962", "7088.905273 5616.130371 357.942535", "6868.194824 4171.091797 174.874771",
			"761.743042 3605.641846 82.054726", "3503.032715 4181.35791 126.241791", "4873.035156 5679.072754 142.983643",
			"6783.110352 4831.280762 127.368523", "5689.485352 5770.709473 316.449738", "8360.750976 5494.712402 17.831593",
			"5924.764648 5012.937988 24.787872", "8371.013672 7563.428223 62.508549", "5521.995605 4904.76416 46.440514",
			"5911.780762 3579.843506 274.124268"
		};
		return spots;
	}

	// Loot points inside the wreck model ("pos|range|height|flags", flags optional).
	protected TStringArray GetCrashLootPoints()
	{
		TStringArray points = {
			"-2.693787 -1.888990 1.671386|0.703328|2.000000|32",
			"-1.758788 -1.888990 -2.302246|0.597443|1.493607|32",
			"-5.150909 -1.888990 -4.171387|1.199951|2.000000|32",
			"-9.951996 -1.888990 -2.661621|0.932135|2.000000|32",
			"-9.645112 -1.888990 1.531249|1.199951|2.000000|32",
			"-7.113404 -1.888990 1.797849|1.199951|2.000000|32",
			"-4.582459 -1.888990 1.906738|1.199951|2.000000|32",
			"-1.599335 -1.888990 -0.949219|0.764948|1.163458|32",
			"-3.084625 -1.796867 -1.247071|0.393962|0.169039|",
			"-4.700715 -1.854195 -1.203125|0.334039|0.211958|",
			"-7.857819 -1.816639 -0.384278|0.407959|1.019897|",
			"-6.361786 -1.474339 -0.424318|0.203125|0.507813|",
			"-5.188050 -1.487791 -0.553956|0.168750|0.421875|",
			"-3.660218 -1.511133 -0.747803|0.134375|0.335938|",
			"-3.550293 -1.555975 -2.536621|0.203125|0.507813|",
			"-6.174928 -1.774635 -1.928222|0.216064|0.540161|",
			"-6.652740 -1.799643 -1.263429|0.603010|0.139997|",
			"-5.446472 -1.813892 -1.698241|0.548840|1.410522|",
			"-2.769746 -1.517029 -2.799561|0.203125|0.507813|",
			"-3.818542 -1.811153 -1.685303|0.460957|1.302490|"
		};
		return points;
	}
}

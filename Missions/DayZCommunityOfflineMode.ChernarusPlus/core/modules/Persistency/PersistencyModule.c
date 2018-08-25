const string BASE_COM_DIR = "$saves:CommunityOfflineMode";
const string BASE_PLAYER_SAVE_DIR = BASE_COM_DIR + "\\PlayerSaves";

class PersistencyModule extends Module
{
	protected ref COMPersistencyScene	m_Scene;

	protected ref COMCharacterMenu 	m_CharacterMenu;
	protected ref COMCharacterSave 	m_CharacterSave;
	protected ref UIScriptedMenu	m_InGameMenu;

	protected bool m_CanBeSaved = false;
	protected bool m_CharacterIsLoaded = false;

	protected string m_sCharacter = "";
	protected string m_sSave = "";

	void PersistencyModule()
	{
		Print("PersistencyModule::PersistencyModule");

		KeyMouseBinding showCharacterMenu = new KeyMouseBinding( GetModuleType() , "ShowCharacterMenu"  , "[M]"    , "Shows the character menu."   );
		showCharacterMenu.AddKeyBind( KeyCode.KC_M, KeyMouseBinding.KB_EVENT_RELEASE );
		RegisterKeyMouseBinding( showCharacterMenu );

		MakeDirectory(BASE_COM_DIR);
		MakeDirectory(BASE_PLAYER_SAVE_DIR);
	}

	void ~PersistencyModule()
	{
		Print("PersistencyModule::~PersistencyModule");

		CleanupCharacterSaving();
		CleanupCharacterMenu();
		CleanupScene();
	}

	string GetLoadedCharacter()
	{
		return m_sCharacter;
	}

	string GetLoadedSave()
	{
		return m_sSave;
	}
	
	override void Init() 
	{
		super.Init();

		#ifdef MODULE_OVERRIDEMENUS
		OverrideMenus om = GetModuleManager().GetModuleByName("OverrideMenus");

		if ( om )
		{
			om.AddPauseButton( new CustomPauseButton( "SAVE CHARACTER", 47842, GetModuleType(), "OpenCharacterSaving" ), new OverrideValid(true, true), 1 );
			om.AddPauseButton( new CustomPauseButton( "LOAD CHARACTER", 47844, GetModuleType(), "OpenCharacterLoading" ), new OverrideValid(true, true), 1 );
		}

		#endif
	}

	override void onMissionLoaded()
	{
		Print("PersistencyModule::onMissionLoaded");
		
		m_CanPause = false;

		m_Scene = new COMPersistencyScene;
 
		m_CharacterMenu = new COMCharacterMenu( this, true );

		GetGame().GetUIManager().ShowScriptedMenu( m_CharacterMenu , NULL );
	}
	
	override void onUpdate( int timeslice ) 
	{
	}

	override void onMissionFinish()
	{
		SavePlayer();
	}

	ref COMPersistencyScene GetScene()
	{
		return m_Scene;
	}

	void CleanupScene()
	{
		Print("PersistencyModule::CleanupScene");

		if (m_Scene)
		{
			delete m_Scene;
		}
	}

	void CleanupCharacterMenu()
	{
		Print("PersistencyModule::CleanupCharacterMenu");

		if (m_CharacterMenu)
		{
			m_CharacterMenu.Close();

			GetGame().GetUIManager().HideScriptedMenu( m_CharacterMenu );

			delete m_CharacterMenu;
		}

		m_CanPause = true;
	}
	
	private void SetupCharacterLoading()
	{
		Print("PersistencyModule::SetupCharacterLoading");
		
		m_CanBeSaved = false;
		m_CanPause = false;

		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(this.SavePlayer);

		GetGame().GetUIManager().CloseMenu( MENU_INGAME );

		if ( m_sCharacter != "" && m_sSave != "" )
		{
			CharacterSave.SavePlayer( m_sCharacter, m_sSave, GetPlayer() );
		}

		m_Scene = new COMPersistencyScene;
	}

	void OpenCharacterLoading()
	{
		Print("PersistencyModule::OpenCharacterLoading");

		CleanupCharacterMenu();
		CleanupScene();

		SetupCharacterLoading();
		
		m_CharacterMenu = new COMCharacterMenu( this, true );

		GetGame().GetUIManager().ShowScriptedMenu( m_CharacterMenu , NULL );
	}

	void TemporaryFix_ReloadCharacterMenu()
	{
		Print("PersistencyModule::TemporaryFix_ReloadCharacterMenu");

		if ( m_CharacterMenu )
		{
			GetGame().GetUIManager().HideScriptedMenu( m_CharacterMenu );
		}

		m_CharacterMenu = new COMCharacterMenu( this, true );

		GetGame().GetUIManager().ShowScriptedMenu( m_CharacterMenu , NULL );
	}

	private void CleanupCharacterSaving()
	{
		if (m_CharacterSave)
		{
			m_CharacterSave.Close();
			
			GetGame().GetUIManager().HideScriptedMenu( m_CharacterSave );

			delete m_CharacterSave;
		}
	}

	void OpenCharacterSaving()
	{
		CleanupCharacterSaving();

		m_CharacterSave = new COMCharacterSave( this );

		m_InGameMenu = GetGame().GetUIManager().FindMenu(MENU_INGAME);
		
		GetGame().GetUIManager().ShowScriptedMenu( m_CharacterSave , m_InGameMenu );
	}
	
	void SavePlayer(string sSave = "latest") 
	{
		Print("PersistencyModule::SavePlayer");

		if ( m_CanBeSaved ) {
			m_sSave = sSave;
			CharacterSave.SavePlayer( m_sCharacter, m_sSave, GetPlayer() );
		} else 
		{
			GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(this.SavePlayer);
		}
	}

	void CreatePlayer(string sCharacter, PlayerBase oPlayer)
	{
		Print("PersistencyModule::CreatePlayer");

		m_CanBeSaved = false;

		MakeDirectory(BASE_PLAYER_SAVE_DIR + "\\" + sCharacter);

		m_sCharacter = sCharacter;
		m_sSave = "latest";

		CharacterSave.CreatePlayer( m_sCharacter, oPlayer );
		
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.LoadPlayer, 100, true, m_sCharacter, m_sSave);
	}

	void LoadLast()
	{
		Print("PersistencyModule::LoadLast");

		m_CanBeSaved = false;
		
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.CreatePlayerInt, 100, true, m_sCharacter, m_sSave);
	}

	void LoadPlayer(string sCharacter, string sSave = "latest")
	{
		Print("PersistencyModule::LoadPlayer");

		m_CanBeSaved = false;

		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(this.SavePlayer);
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(this.LoadPlayer);

		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.CreatePlayerInt, 100, true, sCharacter, sSave);
	}

	private void CreatePlayerInt(string sCharacter, string sSave)
	{
		Print("PersistencyModule::CreatePlayerInt");

		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(this.CreatePlayerInt);

		m_sCharacter = sCharacter;
		m_sSave = sSave;

		PlayerBase oPlayer;

		if ( m_sCharacter == "" )
		{
			oPlayer = CreateCustomDefaultCharacter();
		} else 
		{
			if ( m_sSave == "" )
			{
				m_sSave = "latest";
			}

			oPlayer = CharacterLoad.LoadPlayer( m_sCharacter, m_sSave );
		}

		GetGame().SelectPlayer( NULL, oPlayer );

		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.FinishLoadingInt, 100, true);
	}

	private void FinishLoadingInt()
	{
		Print("PersistencyModule::FinishLoadingInt");

		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.SavePlayer, 1000, true);

		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(this.FinishLoadingInt);

        GetGame().GetMission().GetHud().Show(true);

		GetPlayer().MessageStatus("Loaded character \'" + m_sCharacter + "\' with save \'" + m_sSave + "\'");

		CleanupCharacterMenu();
		CleanupScene();
		
		m_CanBeSaved = true;
	}
}
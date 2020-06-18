#if !defined(TEAMTALKSRVDLL_H)
#define TEAMTALKSRVDLL_H

/*
 * BearWare.dk TeamTalk 5 SDK.
 *
 * Copyright 2005-2018, BearWare.dk.
 *
 * Read the License.txt file included with the TeamTalk 5 SDK for
 * terms of use.
 */

#include "TeamTalk.h"

/**
 * @brief Ensure the header and DLL are exactly the same version. To
 * get the version of the loaded DLL call TT_GetVersion(). A remote
 * client's version can be seen in the @a szVersion member of the
 * #User-struct. */

#define TEAMTALK_SERVER_VERSION "5.6.0.4999"

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @brief A server instance.
     *
     * @see TTS_InitTeamTalk() */
    typedef VOID TTSInstance;

    /** @addtogroup servercallbacks
     * @{ */

    /**
     * @brief Callback when a user is requesting to log on to the
     * server.
     *
     * This callback occurs in the context of TT_DoLogin().
     *
     * Register using TTS_RegisterUserLoginCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpClientErrorMsg Error message which should be sent back to
     * user. Set @c nErrorNo to #CMDERR_SUCCESS if user is authorized.
     * @param lpUser The user properties gathered so far.
     * @param lpUserAccount The user account information which should
     * be set for this user. */
    typedef void UserLoginCallback(IN TTSInstance* lpTTSInstance, 
                                   IN VOID* lpUserData,
                                   OUT ClientErrorMsg* lpClientErrorMsg,
                                   IN const User* lpUser, 
                                   IN OUT UserAccount* lpUserAccount);

    /**
     * @brief Callback when a user is requesting to change nickname.
     *
     * This callback occurs in the context of TT_DoChangeNickname().
     *
     * Register using TTS_RegisterUserChangeNickname().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpClientErrorMsg Error message which should be sent back to
     * user. Set @c nErrorNo to #CMDERR_SUCCESS if user is authorized.
     * @param lpUser The user properties.
     * @param szNewNickname The requested nickname. */
    typedef void UserChangeNicknameCallback(IN TTSInstance* lpTTSInstance, 
                                            IN VOID* lpUserData,
                                            OUT ClientErrorMsg* lpClientErrorMsg,
                                            IN const User* lpUser, 
                                            IN const TTCHAR* szNewNickname);

    /**
     * @brief Callback when a user is requesting to change status.
     *
     * This callback occurs in the context of TT_DoChangeStatus().
     *
     * Register using TTS_RegisterUserChangeStatus().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpClientErrorMsg Error message which should be sent back to
     * user. Set @c nErrorNo to #CMDERR_SUCCESS if user is authorized.
     * @param lpUser The user properties.
     * @param nNewStatusMode The requested status mode.
     * @param szNewStatusMsg The requested nickname. */
    typedef void UserChangeStatusCallback(IN TTSInstance* lpTTSInstance, 
                                          IN VOID* lpUserData,
                                          OUT ClientErrorMsg* lpClientErrorMsg,
                                          IN const User* lpUser, 
                                          IN INT32 nNewStatusMode,
                                          IN const TTCHAR* szNewStatusMsg);
    /**
     * @brief Callback when a user is requesting to create a new user
     * account.
     *
     * This callback occurs in the context of TT_DoNewUserAccount().
     *
     * Register using TTS_RegisterUserCreateUserAccountCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpClientErrorMsg Error message which should be sent back to
     * user. Set @c nErrorNo to #CMDERR_SUCCESS if user is authorized.
     * @param lpUser The user's properties.
     * @param lpUserAccount The properties of the user account to be created. */
    typedef void UserCreateUserAccountCallback(IN TTSInstance* lpTTSInstance, 
                                               IN VOID* lpUserData,
                                               OUT ClientErrorMsg* lpClientErrorMsg,
                                               IN const User* lpUser, 
                                               IN const UserAccount* lpUserAccount);
    /**
     * @brief Callback when a user is requesting to delete a user
     * account.
     *
     * This callback occurs in the context of TT_DoDeleteUserAccount().
     *
     * Register using TTS_RegisterUserDeleteUserAccountCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpClientErrorMsg Error message which should be sent back to
     * user. Set @c nErrorNo to #CMDERR_SUCCESS if user is authorized.
     * @param lpUser The properties of the user requesting.
     * @param szUsername The username of the account to delete. */
    typedef void UserDeleteUserAccountCallback(IN TTSInstance* lpTTSInstance, 
                                               IN VOID* lpUserData,
                                               OUT ClientErrorMsg* lpClientErrorMsg,
                                               IN const User* lpUser, 
                                               IN const TTCHAR* szUsername);
    /**
     * @brief Callback when a user is requesting to ban a user.
     *
     * This callback occurs in the context of TT_DoBanUser().
     *
     * Register using TTS_RegisterUserAddServerBanCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpClientErrorMsg Error message which should be sent back to
     * user. Set @c nErrorNo to #CMDERR_SUCCESS if user is authorized.
     * @param lpBanner The properties of the user requesting the ban.
     * @param lpBanee The properties of the user who should be banned. */
    typedef void UserAddServerBanCallback(IN TTSInstance* lpTTSInstance, 
                                          IN VOID* lpUserData,
                                          OUT ClientErrorMsg* lpClientErrorMsg,
                                          IN const User* lpBanner, 
                                          IN const User* lpBanee);
    /**
     * @brief Callback when a user is requesting to ban an IP-address.
     *
     * This callback occurs in the context of TT_DoBanIPAddress().
     *
     * Register using TTS_RegisterUserAddServerBanIPAddressCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpClientErrorMsg Error message which should be sent back to
     * user. Set @c nErrorNo to #CMDERR_SUCCESS if user is authorized.
     * @param lpBanner The properties of the user requesting the ban. This value
     * can be NULL if #ServerProperties @c nMaxLoginAttempts is enabled.
     * @param szIPAddress The IP-address to be banned. */
    typedef void UserAddServerBanIPAddressCallback(IN TTSInstance* lpTTSInstance, 
                                                   IN VOID* lpUserData,
                                                   OUT ClientErrorMsg* lpClientErrorMsg,
                                                   IN const User* lpBanner, 
                                                   IN const TTCHAR* szIPAddress);
    /**
     * @brief Callback when a user is requesting to remove a ban.
     *
     * This callback occurs in the context of TT_DoUnBanUser().
     *
     * Register using TTS_RegisterUserDeleteServerBanCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpClientErrorMsg Error message which should be sent back to
     * user. Set @c nErrorNo to #CMDERR_SUCCESS if user is authorized.
     * @param lpUser The properties of the user doing the request.
     * @param szIPAddress The IP-address to be unbanned. */
    typedef void UserDeleteServerBanCallback(IN TTSInstance* lpTTSInstance, 
                                             IN VOID* lpUserData,
                                             OUT ClientErrorMsg* lpClientErrorMsg,
                                             IN const User* lpUser, 
                                             IN const TTCHAR* szIPAddress);

    /** @} */

    /** @addtogroup serverlogevents
     * @{ */

    /**
     * @brief Callback when a new user is connecting to the server.
     *
     * Register using TTS_RegisterUserConnectedCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpUser The user properties gathered so far. */
    typedef void UserConnectedCallback(IN TTSInstance* lpTTSInstance,
                                       IN VOID* lpUserData, IN const User* lpUser);
    /**
     * @brief Callback when a user has logged in.
     *
     * This callback occurs in the context of TT_DoLogin() and if
     * #UserLoginCallback returned #CMDERR_SUCCESS.
     *
     * Register using TTS_RegisterUserLoggedInCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpUser The user properties of the user who logged in. */
    typedef void UserLoggedInCallback(IN TTSInstance* lpTTSInstance,
                                      IN VOID* lpUserData, IN const User* lpUser);
    /**
     * @brief Callback when a user has logged out.
     *
     * Register using TTS_RegisterUserLoggedInCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpUser The properties of the user.   */
    typedef void UserLoggedOutCallback(IN TTSInstance* lpTTSInstance,
                                       IN VOID* lpUserData, IN const User* lpUser);
    /**
     * @brief Callback when user has disconnected.
     *
     * Register using TTS_RegisterUserDisconnectedCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpUser The properties of the user.   */
    typedef void UserDisconnectedCallback(IN TTSInstance* lpTTSInstance,
                                          IN VOID* lpUserData, IN const User* lpUser);
    /**
     * @brief Callback when a user's connection has timed out.
     *
     * Register using TTS_RegisterUserTimedoutCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpUser The properties of the user. */
    typedef void UserTimedoutCallback(IN TTSInstance* lpTTSInstance,
                                      IN VOID* lpUserData, IN const User* lpUser);
    /**
     * @brief Callback when a user has been kicked.
     *
     * Register using TTS_RegisterUserKickedCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpKicker The user who had initiated the kick. This can be 0.
     * @param lpKickee The user who has been kicked.
     * @param lpChannel The channel where the user is kicked from. The can be 0. */
    typedef void UserKickedCallback(IN TTSInstance* lpTTSInstance,
                                    IN VOID* lpUserData, IN const User* lpKicker,
                                    IN const User* lpKickee, IN const Channel* lpChannel);
    /**
     * @brief Callback when a user has been banned.
     *
     * Register using TTS_RegisterUserBannedCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpBanner The user who had initiated the ban. This can be 0.
     * @param lpBanee The user who has been banned. This may only
     * contain an IP-address.
     * @param lpChannel The channel where the user is banned from. The can be 0. */
    typedef void UserBannedCallback(IN TTSInstance* lpTTSInstance,
                                    IN VOID* lpUserData, IN const User* lpBanner,
                                    IN const User* lpBanee, IN const Channel* lpChannel);
    /**
     * @brief Callback when a ban is removed.
     *
     * This callback occurs in the contect of TT_DoUnBanUser().
     *
     * Register using TTS_RegisterUserUnbannedCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpUnbanner The user removing the ban.
     * @param szIPAddress The IP-address which is unbanned.   */
    typedef void UserUnbannedCallback(IN TTSInstance* lpTTSInstance,
                                      IN VOID* lpUserData, IN const User* lpUnbanner,
                                      IN const TTCHAR* szIPAddress);
    /**
     * @brief Callback when a user's properties are being updated.
     *
     * Register using TTS_RegisterUserUpdatedCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpUser The properties of the user.   */
    typedef void UserUpdatedCallback(IN TTSInstance* lpTTSInstance,
                                     IN VOID* lpUserData, IN const User* lpUser);
    /**
     * @brief Callback when a user has joined a channel.
     *
     * Register using TTS_RegisterUserJoinedChannelCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpUser The properties of the user.
     * @param lpChannel The properties of the channel being joined.   */
    typedef void UserJoinedChannelCallback(IN TTSInstance* lpTTSInstance,
                                           IN VOID* lpUserData, IN const User* lpUser,
                                           IN const Channel* lpChannel);
    /**
     * @brief Callback when a user has left a channel.
     *
     * Register using TTS_RegisterUserLeftChannelCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpUser The properties of the user.
     * @param lpChannel The properties of the channel being left. */
    typedef void UserLeftChannelCallback(IN TTSInstance* lpTTSInstance,
                                         IN VOID* lpUserData, IN const User* lpUser,
                                         IN const Channel* lpChannel);
    /**
     * @brief Callback when a user has been moved.
     *
     * This callback occurs in the context of TT_DoMoveUser().
     *
     * Register using TTS_RegisterUserMovedCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpMover The user who initiated the move.
     * @param lpMovee The user who has been moved. */
    typedef void UserMovedCallback(IN TTSInstance* lpTTSInstance,
                                   IN VOID* lpUserData, IN const User* lpMover,
                                   IN const User* lpMovee);
    /**
     * @brief Callback when a user is sending a text message.
     *
     * This callback occurs in the context of TT_DoTextMessage().
     *
     * Register using TTS_RegisterUserTextMessageCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpUser The properties of the user.
     * @param lpTextMessage The text message being sent.   */
    typedef void UserTextMessageCallback(IN TTSInstance* lpTTSInstance,
                                         IN VOID* lpUserData, IN const User* lpUser,
                                         IN const TextMessage* lpTextMessage);
    /**
     * @brief Callback when a new channel has been created.
     *
     * This callback occurs in the context of TT_DoMakeChannel() or
     * TT_DoJoinChannel().
     *
     * Register using TTS_RegisterChannelCreatedCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpChannel The channel which has been created.
     * @param lpUser The user who created the channel. This can be 0.   */
    typedef void ChannelCreatedCallback(IN TTSInstance* lpTTSInstance,
                                        IN VOID* lpUserData, IN const Channel* lpChannel,
                                        IN const User* lpUser);
    /**
     * @brief Callback when a channel has been updated.
     *
     * This callback occurs in the context of TT_DoUpdateChannel().
     *
     * Register using TTS_RegisterChannelUpdatedCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpChannel The new properties of the channel.
     * @param lpUser The user who initiated the update. This can be 0.   */
    typedef void ChannelUpdatedCallback(IN TTSInstance* lpTTSInstance,
                                        IN VOID* lpUserData, IN const Channel* lpChannel,
                                        IN const User* lpUser);
    /**
     * @brief Callback when channel has been removed.
     *
     * Register using TTS_RegisterChannelRemovedCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpChannel The properties of the channel which has been removed.
     * @param lpUser The properties of the who initiated the
     * removal. This can be 0.  */
    typedef void ChannelRemovedCallback(IN TTSInstance* lpTTSInstance,
                                        IN VOID* lpUserData, IN const Channel* lpChannel,
                                        IN const User* lpUser);
    /**
     * @brief Callback when a new file has been uploaded to a channel.
     *
     * Register using TTS_RegisterFileUploadedCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpRemoteFile The properties of the file.
     * @param lpUser The properties of the user who uploaded the file. */
    typedef void FileUploadedCallback(IN TTSInstance* lpTTSInstance,
                                      IN VOID* lpUserData, 
                                      IN const RemoteFile* lpRemoteFile,
                                      IN const User* lpUser);
    /**
     * @brief Callback when a user has downloaded a file.
     *
     * Register using TTS_RegisterFileDownloadedCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpRemoteFile The properties of the file.
     * @param lpUser The properties of the user who downloaded the file. */
    typedef void FileDownloadedCallback(IN TTSInstance* lpTTSInstance,
                                        IN VOID* lpUserData, 
                                        IN const RemoteFile* lpRemoteFile,
                                        IN const User* lpUser);
    /**
     * @brief Callback when a user has deleted a file.
     *
     * Register using TTS_RegisterFileDeletedCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpRemoteFile The properties of the file.
     * @param lpUser The properties of the user who deleted the file.   */
    typedef void FileDeletedCallback(IN TTSInstance* lpTTSInstance,
                                     IN VOID* lpUserData, 
                                     IN const RemoteFile* lpRemoteFile,
                                     IN const User* lpUser);
    /**
     * @brief Callback when a user has updated the server properties.
     *
     * This callback occurs in the context of TT_DoUpdateServer().
     *
     * Register using TTS_RegisterServerUpdatedCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpServerProperties The properties of the server.
     * @param lpUser The user who initiated the server update.   */
    typedef void ServerUpdatedCallback(IN TTSInstance* lpTTSInstance,
                                       IN VOID* lpUserData, 
                                       IN const ServerProperties* lpServerProperties,
                                       IN const User* lpUser);
    /**
     * @brief Callback when a user has reguested to save the server
     * configuration.
     *
     * Register using TTS_RegisterSaveServerConfigCallback().
     *
     * @param lpTTSInstance The server instance where the event is occurring.
     * @param lpUserData The user data supplied to register-callback function.
     * @param lpUser The properties of the user who requested to save
     * the server configuration. This can be 0.  */
    typedef void SaveServerConfigCallback(IN TTSInstance* lpTTSInstance,
                                          IN VOID* lpUserData, 
                                          IN const User* lpUser);

    /** @} */

    /** @addtogroup serverapi
     * @{ */

    /**
     * @brief Set certificate and private key for encrypted server.
     *
     * @verbatim
     * NOTE: AT THE MOMENT CALL TTS_SetEncryptionContext() BEFORE
     * CREATING THE SERVER INSTANCE, TTS_InitTeamTalk(). JUST PASS 0
     * AS lpTTSInstance. IN OTHER WORDS ONLY ONE ENCRYPTION CONTEXT IS
     * SUPPORTED AT THE MOMENT.
     * @endverbatim
     *
     * The encrypted server's certificate and private key must be set
     * prior to starting the server using TTS_StartServer().
     *
     * Look in @ref serversetup on how to generate the certificate and
     * private key file using OpenSSL.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param szCertificateFile Path to server's certificate file. 
     * @param szPrivateKeyFile Path to server's private key file. */
    TEAMTALKDLL_API TTBOOL TTS_SetEncryptionContext(IN TTSInstance* lpTTSInstance,
                                                    IN const TTCHAR* szCertificateFile,
                                                    IN const TTCHAR* szPrivateKeyFile);

    /**
     * @brief Create new TeamTalk server instance.
     * 
     * Once server instance is created call TTS_UpdateServer() to set
     * the server's properties followed by TTS_MakeChannel() to create
     * the root channel.
     *
     * @verbatim
     * NOTE: AT THE MOMENT CALL TTS_SetEncryptionContext() BEFORE
     * CREATING THE SERVER INSTANCE, TTS_InitTeamTalk(). JUST PASS 0
     * AS lpTTSInstance. IN OTHER WORDS ONLY ONE ENCRYPTION CONTEXT IS
     * SUPPORTED AT THE MOMENT.
     * @endverbatim
     *
     * @see TTS_StartServer() */
    TEAMTALKDLL_API TTSInstance* TTS_InitTeamTalk();

    /**
     * @brief Close TeamTalk server instance.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     *
     * @see TTS_InitTeamTalk()
     * @see TTS_StartServer() */
    TEAMTALKDLL_API TTBOOL TTS_CloseTeamTalk(IN TTSInstance* lpTTSInstance);

    /**
     * @brief Run the server's event loop.
     * 
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param pnWaitMs The amount of time to wait for the event. If NULL or -1
     * the function will block forever or until the next event occurs.
     * @return Returns TRUE if an event has occured otherwise FALSE.
     * @see TTS_InitTeamTalk() */
    TEAMTALKDLL_API TTBOOL TTS_RunEventLoop(IN TTSInstance* lpTTSInstance,
                                            IN INT32* pnWaitMs);

    /**
     * @brief The root folder of where users should upload files to.
     *
     * The root file folder cannot be changed after the server has
     * been started.
     *
     * Ensure to set #USERRIGHT_UPLOAD_FILES and #USERRIGHT_DOWNLOAD_FILES
     * in user's #UserAccount.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param szFilesRoot Directory where to store uploaded files.
     * @param nMaxDiskUsage The maximum number of bytes which can be used for
     * file storage.
     * @param nDefaultChannelQuota The number of bytes available to temporary
     * channels (not #CHANNEL_PERMANENT). This will be the value in #Channel
     * @c nDiskQuota.
     *
     * @return Error code from #ClientError. */
    TEAMTALKDLL_API INT32 TTS_SetChannelFilesRoot(IN TTSInstance* lpTTSInstance,
                                                  IN const TTCHAR* szFilesRoot,
                                                  IN INT64 nMaxDiskUsage,
                                                  IN INT64 nDefaultChannelQuota);

    /**
     * @brief Set server properties.
     *
     * Set server's properties, like e.g. maximum number of users,
     * server name, etc.
     *
     * Server properties must be set prior to starting a server.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpServerProperties The server's properties which will be
     * see by all users who log on to the server.
     * @return Returns a #ClientError.
     *
     * @see TTS_StartServer() */
    TEAMTALKDLL_API INT32 TTS_UpdateServer(IN TTSInstance* lpTTSInstance,
                                           IN const ServerProperties* lpServerProperties);

    /**
     * @brief Make new channel.
     *
     * Create a new channel on the server. Before starting a server
     * using TTS_StartServer() the server MUST have a root
     * channel. I.e. a #Channel where @c nParentID is 0.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpChannel The new channel to create.
     * @return Returns a #ClientError.
     *
     * @see TTS_UpdateChannel()
     * @see TTS_RemoveChannel() */
    TEAMTALKDLL_API INT32 TTS_MakeChannel(IN TTSInstance* lpTTSInstance,
                                          IN const Channel* lpChannel);

    /**
     * @brief Update an existing channel.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpChannel The updated channel properties. @c nChannelID
     * and @c nParentID must remain the same.
     * @return Returns a #ClientError.
     *
     * @see TTS_MakeChannel()
     * @see TTS_RemoveChannel() */
    TEAMTALKDLL_API INT32 TTS_UpdateChannel(IN TTSInstance* lpTTSInstance,
                                            IN const Channel* lpChannel);

    /**
     * @brief Remove a channel.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param nChannelID The ID of the channel to remove.
     * @return Returns a #ClientError.
     *
     * @see TTS_MakeChannel()
     * @see TTS_UpdateChannel() */
    TEAMTALKDLL_API INT32 TTS_RemoveChannel(IN TTSInstance* lpTTSInstance,
                                            IN INT32 nChannelID);

    /**
     * @brief Add a file to an existing channel.
     *
     * Ensure to have set up file storage first using TTS_SetChannelFilesRoot().
     * Also ensure #Channel's @c nDiskQuota is specified.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param szLocalFilePath Path to file.
     * @param lpRemoteFile Properties of file to add.
     * @return Command error code from #ClientError.
     *
     * @see TTS_SetChannelFilesRoot().
     * @see TTS_MakeChannel() */
    TEAMTALKDLL_API INT32 TTS_AddFileToChannel(IN TTSInstance* lpTTSInstance,
                                               IN const TTCHAR* szLocalFilePath,
                                               IN const RemoteFile* lpRemoteFile);

    /**
     * @brief Remove a file from a channel.
     *
     * Ensure to have set up file storage first using TTS_SetChannelFilesRoot().
     * Also ensure #Channel's @c nDiskQuota is specified.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpRemoteFile Properties of file to remove. Channel ID and 
     * file name is enough.
     * @return Command error code from #ClientError.
     *
     * @see TTS_SetChannelFilesRoot().
     * @see TTS_MakeChannel() */
    TEAMTALKDLL_API INT32 TTS_RemoveFileFromChannel(IN TTSInstance* lpTTSInstance,
                                                    IN const RemoteFile* lpRemoteFile);

    /**
     * @brief Move a user from one channel to another.
     *
     * 
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param nUserID The ID of the user to move.
     * @param lpChannel The channel the user should move to. If the
     * channel already exists then simply set @c nChannelID. To make
     * a user leave a channel set @c nChannelID to 0.
     * @return Returns a #ClientError.
     * @see TT_DoMoveUser() */
    TEAMTALKDLL_API INT32 TTS_MoveUser(IN TTSInstance* lpTTSInstance,
                                       IN INT32 nUserID, IN const Channel* lpChannel);

    /**
     * @brief Send text message from server to clients.
     *
     * 
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpTextMessage Text message to send. The message type determines
     * how the message will be sent.
     * @return Returns a #ClientError.
     * @see TT_DoTextMessage() */
    TEAMTALKDLL_API INT32 TTS_SendTextMessage(IN TTSInstance* lpTTSInstance,
                                              const TextMessage* lpTextMessage);

    /** 
     * @brief Start server on specified IP-address and ports.
     *
     * Before starting a server the root channel must be created using
     * TTS_MakeChannel().
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param szBindIPAddr The IP-address to bind to.
     * @param nTcpPort The TCP port to bind to.
     * @param nUdpPort The UDP port to bind to.
     * @param bEncrypted If encryption is enabled then encryption context
     * must be set prior to this call using TTS_SetEncryptionContext().
     *
     * @see TTS_SetEncryptionContext()
     * @see TTS_MakeChannel() */
    TEAMTALKDLL_API TTBOOL TTS_StartServer(IN TTSInstance* lpTTSInstance,
                                           IN const TTCHAR* szBindIPAddr, 
                                           IN UINT16 nTcpPort, 
                                           IN UINT16 nUdpPort,
                                           IN TTBOOL bEncrypted);

    /**
     * @brief Same as TTS_StartServer() but with the option of
     * specifying a system-ID.
     *
     * Requires TeamTalk version 5.1.3.4506.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param szBindIPAddr The IP-address to bind to.
     * @param nTcpPort The TCP port to bind to.
     * @param nUdpPort The UDP port to bind to.
     * @param bEncrypted If encryption is enabled then encryption context
     * must be set prior to this call using TTS_SetEncryptionContext().
     * @param szSystemID The identification of the conferencing system.
     * The default value is "teamtalk". See TT_ConnectSysID(). */
    TEAMTALKDLL_API TTBOOL TTS_StartServerSysID(IN TTSInstance* lpTTSInstance,
                                                IN const TTCHAR* szBindIPAddr, 
                                                IN UINT16 nTcpPort, 
                                                IN UINT16 nUdpPort,
                                                IN TTBOOL bEncrypted, 
                                                IN const TTCHAR* szSystemID);
    

    /**
     * @brief Stop server and drop all users.
     *
     * @see TTS_StartServer() */
    TEAMTALKDLL_API TTBOOL TTS_StopServer(IN TTSInstance* lpTTSInstance);

    /** @} */


    /** @addtogroup servercallbacks
     * @{ */

    /**
     * @brief Register a callback when a user is requesting to log on
     * to the server.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterUserLoginCallback(IN TTSInstance* lpTTSInstance,
                                                         IN UserLoginCallback* lpCallback,
                                                         IN VOID* lpUserData, IN TTBOOL bEnable);
                                                      
    /**
     * @brief Register a callback to when user is changing nickname.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful.
     * @see TT_DoChangeNickname() */
    TEAMTALKDLL_API TTBOOL TTS_RegisterUserChangeNicknameCallback(IN TTSInstance* lpTTSInstance,
                                                                  IN UserChangeNicknameCallback* lpCallback,
                                                                  IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback to when user is changing status.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful.
     * @see TT_DoChangeStatus() */
    TEAMTALKDLL_API TTBOOL TTS_RegisterUserChangeStatusCallback(IN TTSInstance* lpTTSInstance,
                                                                IN UserChangeStatusCallback* lpCallback,
                                                                IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when a user is requesting to create a
     * new user account on the server.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterUserCreateUserAccountCallback(IN TTSInstance* lpTTSInstance,
                                                                     IN UserCreateUserAccountCallback* lpCallback,
                                                                     IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when a user is requesting to delete
     * a user account on the server.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterUserDeleteUserAccountCallback(IN TTSInstance* lpTTSInstance,
                                                                     IN UserDeleteUserAccountCallback* lpCallback,
                                                                     IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when a user is requesting to add a
     * server ban requested by a user.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterUserAddServerBanCallback(IN TTSInstance* lpTTSInstance,
                                                                IN UserAddServerBanCallback* lpCallback,
                                                                IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when a user is requesting to add a
     * server IP-address ban requested by a user.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterUserAddServerBanIPAddressCallback(IN TTSInstance* lpTTSInstance,
                                                                         IN UserAddServerBanIPAddressCallback* lpCallback,
                                                                         IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when a user is requesting to delete
     * a server IP-address ban requested by a user.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterUserDeleteServerBanCallback(IN TTSInstance* lpTTSInstance,
                                                                   IN UserDeleteServerBanCallback* lpCallback,
                                                                   IN VOID* lpUserData, IN TTBOOL bEnable);


    /** @} */

    /** @addtogroup serverlogevents
     * @{ */

    /**
     * @brief Register a callback when a user connects to the server.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterUserConnectedCallback(IN TTSInstance* lpTTSInstance,
                                                             IN UserConnectedCallback* lpCallback,
                                                             IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when a user logs on to the server.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterUserLoggedInCallback(IN TTSInstance* lpTTSInstance,
                                                            IN UserLoggedInCallback* lpCallback,
                                                            IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when a user logs out of the server.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterUserLoggedOutCallback(IN TTSInstance* lpTTSInstance,
                                                             IN UserLoggedOutCallback* lpCallback,
                                                             IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when a user disconnects from the server.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterUserDisconnectedCallback(IN TTSInstance* lpTTSInstance,
                                                                IN UserDisconnectedCallback* lpCallback,
                                                                IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when a user is dropped because of
     * inactivity.
     *
     * See @c nUserTimeout in #ServerProperties.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterUserTimedoutCallback(IN TTSInstance* lpTTSInstance,
                                                            IN UserTimedoutCallback* lpCallback,
                                                            IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when a user is kicked from the server.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterUserKickedCallback(IN TTSInstance* lpTTSInstance,
                                                          IN UserKickedCallback* lpCallback,
                                                          IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when a user is banned from the server.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterUserBannedCallback(IN TTSInstance* lpTTSInstance,
                                                          IN UserBannedCallback* lpCallback,
                                                          IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when an IP-address is unbanned from
     * the server.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterUserUnbannedCallback(IN TTSInstance* lpTTSInstance,
                                                            IN UserUnbannedCallback* lpCallback,
                                                            IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when an user's properties are updated.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterUserUpdatedCallback(IN TTSInstance* lpTTSInstance,
                                                           IN UserUpdatedCallback* lpCallback,
                                                           IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when an user joins a channel.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterUserJoinedChannelCallback(IN TTSInstance* lpTTSInstance,
                                                                 IN UserJoinedChannelCallback* lpCallback,
                                                                 IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when an user leaves a channel.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterUserLeftChannelCallback(IN TTSInstance* lpTTSInstance,
                                                               IN UserLeftChannelCallback* lpCallback,
                                                               IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when a user is moved.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterUserMovedCallback(IN TTSInstance* lpTTSInstance,
                                                         IN UserMovedCallback* lpCallback,
                                                         IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when a user send a text message.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterUserTextMessageCallback(IN TTSInstance* lpTTSInstance,
                                                               IN UserTextMessageCallback* lpCallback,
                                                               IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when a channel is created.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterChannelCreatedCallback(IN TTSInstance* lpTTSInstance,
                                                              IN ChannelCreatedCallback* lpCallback,
                                                              IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when a channel is updated.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterChannelUpdatedCallback(IN TTSInstance* lpTTSInstance,
                                                              IN ChannelUpdatedCallback* lpCallback,
                                                              IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when a channel is removed.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterChannelRemovedCallback(IN TTSInstance* lpTTSInstance,
                                                              IN ChannelRemovedCallback* lpCallback,
                                                              IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when a file is uploaded.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterFileUploadedCallback(IN TTSInstance* lpTTSInstance,
                                                            IN FileUploadedCallback* lpCallback,
                                                            IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when a file is downloaded.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterFileDownloadedCallback(IN TTSInstance* lpTTSInstance,
                                                              IN FileDownloadedCallback* lpCallback,
                                                              IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when a file is deleted.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterFileDeletedCallback(IN TTSInstance* lpTTSInstance,
                                                           IN FileDeletedCallback* lpCallback,
                                                           IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when a server's properties has been updated.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterServerUpdatedCallback(IN TTSInstance* lpTTSInstance,
                                                             IN ServerUpdatedCallback* lpCallback,
                                                             IN VOID* lpUserData, IN TTBOOL bEnable);

    /**
     * @brief Register a callback when a user requests the server to
     * save its configuration.
     *
     * @param lpTTSInstance Pointer to the server instance created by
     * TTS_InitTeamTalk().
     * @param lpCallback Pointer to a function which will handle the callback.
     * @param lpUserData A pointer which will be passed to the callback function.
     * @param bEnable Whether to register or unregister the callback.
     * @return Returns TRUE if the call was successful. */
    TEAMTALKDLL_API TTBOOL TTS_RegisterSaveServerConfigCallback(IN TTSInstance* lpTTSInstance,
                                                                IN SaveServerConfigCallback* lpCallback,
                                                                IN VOID* lpUserData, IN TTBOOL bEnable);

    /** @} */

#ifdef __cplusplus
}
#endif

#endif /** TEAMTALKSRVDLL_H */

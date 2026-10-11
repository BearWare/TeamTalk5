import dk.bearware.BannedUser;
import dk.bearware.UserAccount;


public interface BanProvider {
    // return command ID
    int issueAddBan(BannedUser ban);
    int issueRemoveBan(BannedUser ban);
    UserAccount banOwner();
}
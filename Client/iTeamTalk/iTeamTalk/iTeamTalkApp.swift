import SwiftUI

@main
struct iTeamTalkApp: App {
    @UIApplicationDelegateAdaptor(AppDelegate.self) var appDelegate

    var body: some Scene {
        WindowGroup {
            RootNavigationView()
        }
    }
}

private struct RootNavigationView: UIViewControllerRepresentable {
    func makeUIViewController(context: Context) -> UIViewController {
        UINavigationController(rootViewController: ServerListViewController())
    }

    func updateUIViewController(_ uiViewController: UIViewController, context: Context) {}
}

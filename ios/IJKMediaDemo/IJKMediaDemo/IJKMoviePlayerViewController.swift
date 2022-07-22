//  Converted to Swift 5.5 by Swiftify v5.5.24279 - https://swiftify.com/
/*
 * Copyright (C) 2013-2015 Bilibili
 * Copyright (C) 2013-2015 Zhang Rui <bbcallen@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import IJKMediaFramework
import UIKit

class IJKVideoViewController: UIViewController {
    var url: URL?
    weak var player: IJKMediaPlayback?
    @IBOutlet var mediaControl: IJKMediaControl!

    deinit {
    }

    class func present(from viewController: UIViewController?, withTitle title: String?, url: URL?, completion: @escaping () -> Void) {
        let historyItem = IJKDemoHistoryItem()

        historyItem.title = title
        historyItem.url = url
        IJKDemoHistory.instance().add(historyItem)

        viewController?.present(IJKVideoViewController(url: url), animated: true, completion: completion)
    }

    convenience init(url: URL?) {
        self.init(nibName: "IJKMoviePlayerViewController", bundle: nil)
        self.url = url
    }

    override init(nibName nibNameOrNil: String?, bundle nibBundleOrNil: Bundle?) {
        super.init(nibName: nibNameOrNil, bundle: nibBundleOrNil)
        // Custom initialization
    }

    //#define EXPECTED_IJKPLAYER_VERSION (1 << 16) & 0xFF) |

    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view from its nib.

        //    [[UIApplication sharedApplication] setStatusBarHidden:YES];
        //    [[UIApplication sharedApplication] setStatusBarOrientation:UIInterfaceOrientationLandscapeLeft animated:NO];

        #if DEBUG
        IJKFFMoviePlayerController.logReport = true
        IJKFFMoviePlayerController.logLevel = k_IJK_LOG_DEBUG
        #else
        IJKFFMoviePlayerController.logReport = false
        IJKFFMoviePlayerController.logLevel = k_IJK_LOG_INFO
        #endif

        IJKFFMoviePlayerController.checkIfFFmpegVersionMatch(true)
        // [IJKFFMoviePlayerController checkIfPlayerVersionMatch:YES major:1 minor:0 micro:0];

        let options = IJKFFOptions.byDefault()

        player = IJKFFMoviePlayerController(contentURL: url, with: options)
        player?.view.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        player?.view.frame = view.bounds
        player?.scalingMode = IJKMPMovieScalingModeAspectFit
        player?.shouldAutoplay = true

        view.autoresizesSubviews = true
        if let view = player?.view {
            view.addSubview(view)
        }
        view.addSubview(mediaControl)

        mediaControl.delegatePlayer = player
    }

    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)

        installMovieNotificationObservers()

        player?.prepareToPlay()
    }

    override func viewDidDisappear(_ animated: Bool) {
        super.viewDidDisappear(animated)

        player?.shutdown()
        removeMovieNotificationObservers()
    }

    override var shouldAutorotate: Bool {
        return toInterfaceOrientation.isLandscape
    }

    override var supportedInterfaceOrientations: UIInterfaceOrientationMask {
        return .landscape
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }

    // MARK: IBAction

    @IBAction func onClickMediaControl(_ sender: Any) {
        mediaControl.showAndFade()
    }

    @IBAction func onClickOverlay(_ sender: Any) {
        mediaControl.hide()
    }

    @IBAction func onClickDone(_ sender: Any) {
        presentingViewController?.dismiss(animated: true)
    }

    @IBAction func onClickHUD(_ sender: UIBarButtonItem) {
        if self.player is IJKFFMoviePlayerController {
            let player = self.player as? IJKFFMoviePlayerController
            player?.shouldShowHudView = !(player?.shouldShowHudView)!

            sender.title = player?.shouldShowHudView ? "HUD On" : "HUD Off"
        }
    }

    @IBAction func onClickPlay(_ sender: Any) {
        player?.play()
        mediaControl.refresh()
    }

    @IBAction func onClickPause(_ sender: Any) {
        player?.pause()
        mediaControl.refresh()
    }

    @IBAction func didSliderTouchDown() {
        mediaControl.beginDragMediaSlider()
    }

    @IBAction func didSliderTouchCancel() {
        mediaControl.endDragMediaSlider()
    }

    @IBAction func didSliderTouchUpOutside() {
        mediaControl.endDragMediaSlider()
    }

    @IBAction func didSliderTouchUpInside() {
        player?.currentPlaybackTime = mediaControl.mediaProgressSlider.value
        mediaControl.endDragMediaSlider()
    }

    @IBAction func didSliderValueChanged() {
        mediaControl.continueDragMediaSlider()
    }

    @objc func loadStateDidChange(_ notification: Notification?) {
        //    MPMovieLoadStateUnknown        = 0,
        //    MPMovieLoadStatePlayable       = 1 << 0,
        //    MPMovieLoadStatePlaythroughOK  = 1 << 1, // Playback will be automatically started in this state when shouldAutoplay is YES
        //    MPMovieLoadStateStalled        = 1 << 2, // Playback will be automatically paused in this state, if started

        let loadState = player?.loadState

        if let loadState = loadState {
            if (loadState & IJKMPMovieLoadStatePlaythroughOK) != 0 {
                print("loadStateDidChange: IJKMPMovieLoadStatePlaythroughOK: \(Int(loadState ?? 0))\n")
            } else if (loadState & IJKMPMovieLoadStateStalled) != 0 {
                print("loadStateDidChange: IJKMPMovieLoadStateStalled: \(Int(loadState ?? 0))\n")
            } else {
                print("loadStateDidChange: ???: \(Int(loadState ?? 0))\n")
            }
        }
    }

    @objc func moviePlayBackDidFinish(_ notification: Notification?) {
        //    MPMovieFinishReasonPlaybackEnded,
        //    MPMovieFinishReasonPlaybackError,
        //    MPMovieFinishReasonUserExited
        let reason = (notification?.userInfo?[IJKMPMoviePlayerPlaybackDidFinishReasonUserInfoKey] as? NSNumber)?.intValue ?? 0

        switch reason {
        case Int(IJKMPMovieFinishReasonPlaybackEnded):
            print("playbackStateDidChange: IJKMPMovieFinishReasonPlaybackEnded: \(reason)\n")
        case Int(IJKMPMovieFinishReasonUserExited):
            print("playbackStateDidChange: IJKMPMovieFinishReasonUserExited: \(reason)\n")
        case Int(IJKMPMovieFinishReasonPlaybackError):
            print("playbackStateDidChange: IJKMPMovieFinishReasonPlaybackError: \(reason)\n")
        default:
            print("playbackPlayBackDidFinish: ???: \(reason)\n")
        }
    }

    @objc func mediaIsPrepared(toPlayDidChange notification: Notification?) {
        print("mediaIsPreparedToPlayDidChange\n")
    }

    @objc func moviePlayBackStateDidChange(_ notification: Notification?) {
        //    MPMoviePlaybackStateStopped,
        //    MPMoviePlaybackStatePlaying,
        //    MPMoviePlaybackStatePaused,
        //    MPMoviePlaybackStateInterrupted,
        //    MPMoviePlaybackStateSeekingForward,
        //    MPMoviePlaybackStateSeekingBackward

        switch player?.playbackState {
        case IJKMPMoviePlaybackStateStopped:
            print("IJKMPMoviePlayBackStateDidChange \(Int(player?.playbackState ?? 0)): stoped")
        case IJKMPMoviePlaybackStatePlaying:
            print("IJKMPMoviePlayBackStateDidChange \(Int(player?.playbackState ?? 0)): playing")
        case IJKMPMoviePlaybackStatePaused:
            print("IJKMPMoviePlayBackStateDidChange \(Int(player?.playbackState ?? 0)): paused")
        case IJKMPMoviePlaybackStateInterrupted:
            print("IJKMPMoviePlayBackStateDidChange \(Int(player?.playbackState ?? 0)): interrupted")
        case IJKMPMoviePlaybackStateSeekingForward, IJKMPMoviePlaybackStateSeekingBackward:
            print("IJKMPMoviePlayBackStateDidChange \(Int(player?.playbackState ?? 0)): seeking")
        default:
            print("IJKMPMoviePlayBackStateDidChange \(Int(player?.playbackState ?? 0)): unknown")
        }
    }

    // MARK: Install Movie Notifications

    // Register observers for the various movie object notifications.
    func installMovieNotificationObservers() {
        NotificationCenter.default.addObserver(
            self,
            selector: #selector(loadStateDidChange(_:)),
            name: IJKMPMoviePlayerLoadStateDidChangeNotification,
            object: player)

        NotificationCenter.default.addObserver(
            self,
            selector: #selector(moviePlayBackDidFinish(_:)),
            name: IJKMPMoviePlayerPlaybackDidFinishNotification,
            object: player)

        NotificationCenter.default.addObserver(
            self,
            selector: #selector(mediaIsPrepared(toPlayDidChange:)),
            name: IJKMPMediaPlaybackIsPreparedToPlayDidChangeNotification,
            object: player)

        NotificationCenter.default.addObserver(
            self,
            selector: #selector(moviePlayBackStateDidChange(_:)),
            name: IJKMPMoviePlayerPlaybackStateDidChangeNotification,
            object: player)
    }

    // MARK: Remove Movie Notification Handlers

    // Remove the movie notification observers from the movie object.
    func removeMovieNotificationObservers() {
        NotificationCenter.default.removeObserver(self, name: IJKMPMoviePlayerLoadStateDidChangeNotification, object: player)
        NotificationCenter.default.removeObserver(self, name: IJKMPMoviePlayerPlaybackDidFinishNotification, object: player)
        NotificationCenter.default.removeObserver(self, name: IJKMPMediaPlaybackIsPreparedToPlayDidChangeNotification, object: player)
        NotificationCenter.default.removeObserver(self, name: IJKMPMoviePlayerPlaybackStateDidChangeNotification, object: player)
    }

    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
    }
}
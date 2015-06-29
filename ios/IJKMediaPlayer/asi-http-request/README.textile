ASIHTTPRequest is an easy to use wrapper around the CFNetwork API that makes some of the more tedious aspects of communicating with web servers easier. It is written in Objective-C and works in both Mac OS X and iPhone applications.

It is suitable performing basic HTTP requests and interacting with REST-based services (GET / POST / PUT / DELETE). The included ASIFormDataRequest subclass makes it easy to submit POST data and files using multipart/form-data.

It provides:
* A straightforward interface for submitting data to and fetching data from webservers
* Download data to memory or directly to a file on disk
* Submit files on local drives as part of POST data, compatible with the HTML file input mechanism
* Stream request bodies directly from disk to the server, to conserve memory
* Resume for partial downloads
* Easy access to request and response HTTP headers
* Progress delegates (NSProgressIndicators and UIProgressViews) to show information about download AND upload progress
* Auto-magic management of upload and download progress indicators for operation queues
* Basic, Digest + NTLM authentication support, credentials are automatically re-used for the duration of a session, and can be stored for later in the Keychain.
* Cookie support
* [NEW] Requests can continue to run when your app moves to the background (iOS 4+)
* GZIP support for response data AND request bodies
* The included ASIDownloadCache class lets requests transparently cache responses, and allow requests for cached data to succeed even when there is no network available
* [NEW] ASIWebPageRequest - download complete webpages, including external resources like images and stylesheets. Pages of any size can be indefinitely cached, and displayed in a UIWebview / WebView even when you have no network connection.
* Easy to use support for Amazon S3 - no need to fiddle around signing requests yourself!
* Full support for Rackspace Cloud Files
* [NEW] Client certificates support
* Supports manual and auto-detected proxies, authenticating proxies, and PAC file auto-configuration. The built-in login dialog lets your iPhone application work transparently with authenticating proxies without any additional effort.
* Bandwidth throttling support
* Support for persistent connections
* Supports synchronous & asynchronous requests
* Get notifications about changes in your request state via delegation or [NEW] blocks (Mac OS X 10.6, iOS 4 and above)
* Comes with a broad range of unit tests

ASIHTTPRequest is compatible with Mac OS 10.5 or later, and iOS 3.0 or later.

Documentation is available "here":http://allseeing-i.com/ASIHTTPRequest.
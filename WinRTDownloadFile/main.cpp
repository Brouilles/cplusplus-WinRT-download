#include "pch.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Web::Http;
using namespace Windows::Web::Http::Filters;
using namespace Windows::Web::Http::Headers;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage;

HttpClient httpClient{ nullptr };

IAsyncActionWithProgress<int> CancelableRequestAsync(HttpRequestMessage request)
{
    //auto lifetime = get_strong();
    // auto lifetime = winrt::implements::get_strong();
    //auto cancellation = co_await get_cancellation_token();
    //cancellation.enable_propagation();
    auto progress = co_await get_progress_token();

    // Do not buffer the response.
    HttpRequestResult result = co_await httpClient.TrySendRequestAsync(request, HttpCompletionOption::ResponseHeadersRead);

    if (result.Succeeded())
    {
        // std::wostringstream responseBody;
        std::fstream file;
        file.open("server.jar", std::ios::app | std::ios::binary);
        //file.open("robots.txt", std::ios::app | std::ios::binary);

        bool a = true;

        //DataWriter writer;
        IInputStream responseStream = co_await result.ResponseMessage().Content().ReadAsInputStreamAsync();
        Buffer readBuffer(1000);
        IBuffer resultBuffer;
        do
        {
            resultBuffer = co_await responseStream.ReadAsync(readBuffer, readBuffer.Capacity(), InputStreamOptions::Partial);
            //std::cout << "Bytes read from stream: " << resultBuffer.Length() << std::endl;
            progress(resultBuffer.Length());

            //hstring resultText = CryptographicBuffer::EncodeToHexString(resultBuffer);
            //file.write(reinterpret_cast<char*>(resultBuffer.data()), resultBuffer.Capacity());

        } while (resultBuffer.Length() > 0);
        responseStream.Close();
        file.close();

        printf("Completed\n");
        httpClient.Close();
    }
    else
    {
        printf("Error in CancelableRequestAsync\n");
        // result.ExtendedError()
    }

    co_return;
}

struct DownloadFile : implements<DownloadFile, IInspectable>
{
    IAsyncAction Async(HttpRequestMessage request)
    {
        auto lifetime = get_strong(); // <-- keep alive

       // Do not buffer the response.
        HttpRequestResult result = co_await httpClient.TrySendRequestAsync(request, HttpCompletionOption::ResponseHeadersRead);

        if (result.Succeeded())
        {
            /*
            * 
            * wil::max_extended_path_length comes from https://github.com/microsoft/wil/blob/23fca640eae099f9120545483ae7d5d0a98a6fa5/include/wil/win32_helpers.h#L51
            * you can either install that library (it's on nuget), or copy-paste size_t const max_extended_path_length = 0x7FFF - 24; somewhere in your code
            * 
            std::filesystem::path GetProcessFileName(HANDLE process)
            {
                std::wstring exeLocation;
                exeLocation.resize(wil::max_extended_path_length);

                DWORD exeLocation_size = static_cast<DWORD>(exeLocation.size()) + 1;
                winrt::check_bool(QueryFullProcessImageName(process, 0, exeLocation.data(), &exeLocation_size));

                exeLocation.resize(exeLocation_size);
                return exeLocation;
            }

            std::filesystem::path GetExeLocation()
            {
                return GetProcessFileName(GetCurrentProcess());
            }

            auto folder = co_await StorageFolder::GetFolderFromPathAsync(GetExeLocation().parent_path().native());
            */
            hstring fileName = L"server.jar";
            //You will need to call QueryFullProcessImageName, then strip out the .exe from that using std::filesystem::path
            StorageFolder localFolder = co_await StorageFolder::GetFolderFromPathAsync(L"E:\\Git\\cplusplus-WinRT-download\\Files");
            StorageFile storageFile = co_await localFolder.CreateFileAsync(fileName, winrt::Windows::Storage::CreationCollisionOption::ReplaceExisting);
            IRandomAccessStream fileStream = co_await storageFile.OpenAsync(FileAccessMode::ReadWrite);
            IInputStream responseStream = co_await result.ResponseMessage().Content().ReadAsInputStreamAsync();
            
            co_await RandomAccessStream::CopyAsync(responseStream, fileStream);
            /*co_await RandomAccessStream::CopyAsync(responseStream, fileStream)
                .Progress([](auto&& sender, auto progress) { std::cout << "Bytes read from stream Progress: " << progress << std::endl; });*/


            printf("Completed\n");
            httpClient.Close();
        }
        else
        {
            printf("Error in CancelableRequestAsync\n");
            // result.ExtendedError()
        }

        co_return;
    }
};

//IAsyncActionWithProgress StartDownloadFile()
void StartDownloadFile()
{
    //Windows::Foundation::IAsyncAction pendingAction{ nullptr };

    httpClient = HttpClient();

    //Uri resourceUri{ L"https://releases.ubuntu.com/20.04.3/ubuntu-20.04.3-desktop-amd64.iso" };
    Uri resourceUri{ L"https://launcher.mojang.com/v1/objects/125e5adf40c659fd3bce3e66e67a16bb49ecc1b9/server.jar" };
    //Uri resourceUri{ L"https://www.dezeiraud.com/robots.txt" };
    HttpRequestMessage request(HttpMethod::Get(), resourceUri);

    auto down = make_self<DownloadFile>();
    down->Async(request).get();

    /*
    try
    {
        auto down = make_self<DownloadFile>();
        down->request = request;
        down->Async().get();
        // pendingAction = CancelableRequestAsync(request);
        //CancelableRequestAsync(request).get();
        //auto action = CancelableRequestAsync(request);
        //action.Progress([](auto&& sender, auto progress) { std::cout << "Bytes read from stream Progress: " << progress << std::endl; });
        //co_await action;
        //co_await pendingAction;
    }
    catch (hresult_canceled const&)
    {
        printf("Request canceled\n");
    }
    */
}

int main()
{
    init_apartment();
    StartDownloadFile();
}

#include <JuceHeader.h>

// Represents a single download task
class DownloadTask : public ThreadPoolJob
{
public:
    DownloadTask(const URL& urlToDownload, const File& destinationFile)
        : ThreadPoolJob("Download: " + urlToDownload.toString(false)),
          url(urlToDownload),
          destination(destinationFile)
    {
    }

    JobStatus runJob() override
    {
        // Perform download in worker thread
        std::unique_ptr<InputStream> input(url.createInputStream(URL::InputStreamOptions(URL::ParameterHandling::inAddress)
            .withConnectionTimeoutMs(5000)
            .withNumRedirectsToFollow(5)));

        if (input == nullptr)
        {
            status = "Failed to create input stream";
            return jobHasFinished;
        }

        if (!destination.create())
        {
            status = "Failed to create destination file";
            return jobHasFinished;
        }

        FileOutputStream output(destination);
        if (!output.openedOk())
        {
            status = "Failed to open output stream";
            return jobHasFinished;
        }

        int64 totalBytes = input->getTotalLength();
        int64 bytesDownloaded = 0;
        const int bufferSize = 8192;
        HeapBlock<char> buffer(bufferSize);

        while (!input->isExhausted() && !shouldExit())
        {
            int bytesRead = input->read(buffer.getData(), bufferSize);
            if (bytesRead <= 0)
                break;

            output.write(buffer.getData(), bytesRead);
            bytesDownloaded += bytesRead;

            // Update progress (thread-safe)
            {
                ScopedLock lock(progressLock);
                progress = totalBytes > 0 ? static_cast<float>(bytesDownloaded) / totalBytes : 0.0f;
            }
        }

        output.flush();
        status = input->isExhausted() ? "Completed" : "Interrupted";
        return jobHasFinished;
    }

    float getProgress() const
    {
        ScopedLock lock(progressLock);
        return progress;
    }

    String getStatus() const { return status; }
    URL getURL() const { return url; }
    File getDestination() const { return destination; }

private:
    URL url;
    File destination;
    float progress = 0.0f;
    String status = "Pending";
    CriticalSection progressLock;
};

// Manages the download queue and UI updates
class DownloadManager : public Component, public Timer
{
public:
    DownloadManager()
    {
        // Use half the available cores, minimum 1
        int numThreads = jmax(1, SystemStats::getNumCpus() / 2);
        threadPool = std::make_unique<ThreadPool>(numThreads);
        
        startTimerHz(30); // Update UI at 30 FPS
    }

    ~DownloadManager() override
    {
        threadPool->removeAllJobs(true, 5000);
    }

    void addDownload(const URL& url, const File& destination)
    {
        auto* task = new DownloadTask(url, destination);
        downloads.add(task);
        threadPool->addJob(task, true);
        updateDownloadList();
    }

    void paint(Graphics& g) override
    {
        g.fillAll(Colours::white);
        g.setColour(Colours::black);
        
        int y = 10;
        for (int i = 0; i < downloads.size(); ++i)
        {
            auto* task = downloads[i];
            String status = task->getStatus();
            float progress = task->getProgress();
            
            g.drawText(task->getURL().toString(false), 10, y, getWidth() - 20, 20, Justification::left);
            g.drawText(status + " (" + String(progress * 100.0f, 1) + "%)", 
                      10, y + 20, getWidth() - 20, 20, Justification::left);
            y += 50;
        }
    }

private:
    void timerCallback() override
    {
        // Periodically clean up completed downloads and refresh UI
        for (int i = downloads.size(); --i >= 0;)
        {
            if (!threadPool->contains(downloads[i]))
                downloads.remove(i);
        }
        
        if (downloads.size() != lastDownloadCount)
            updateDownloadList();
        
        repaint();
    }

    void updateDownloadList()
    {
        lastDownloadCount = downloads.size();
        setSize(400, jmax(100, downloads.size() * 50));
    }

    std::unique_ptr<ThreadPool> threadPool;
    OwnedArray<DownloadTask> downloads;
    int lastDownloadCount = 0;
};

// Example usage in DownloaderComponent
class DownloaderComponent : public Component
{
public:
    DownloaderComponent()
    {
        addAndMakeVisible(downloadManager);
        
        // Example: Queue some downloads
        File downloadDir = File::getSpecialLocation(File::userDesktopDirectory);
        
        downloadManager.addDownload(URL("http://example.com/file1.zip"), 
                                  downloadDir.getChildFile("file1.zip"));
        downloadManager.addDownload(URL("http://example.com/file2.pdf"), 
                                  downloadDir.getChildFile("file2.pdf"));
        
        setSize(400, 200);
    }

    void resized() override
    {
        downloadManager.setBounds(getLocalBounds());
    }

private:
    DownloadManager downloadManager;
};

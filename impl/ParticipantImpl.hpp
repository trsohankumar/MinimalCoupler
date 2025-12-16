#include<string>
#include<vector>

namespace MinimalCoupler
{
class ParticipantImplementation
{
public:
    ParticipantImplementation(
      std::string participantName,
      std::string configurationFileName,
      int         solverProcessIndex,
      int         solverProcessSize);

    virtual ~ParticipantImplementation() = default;

    // Mesh methods
    virtual int getMeshDimensions(const std::string& meshName) const;

    virtual void setMeshVertices(
      const std::string& meshName,
      const std::vector<double>& positions,
      std::vector<int>& ids);

    // Data exchange methods
    virtual void readData(
      const std::string& meshName,
      const std::string& dataName,
      const std::vector<int>& vertexIDs,
      double relativeReadTime,
      std::vector<double>& values) const;

    virtual void writeData(
      const std::string& meshName,
      const std::string& dataName,
      const std::vector<int>& vertexIDs,
      const std::vector<double>& values);

    // Steering methods
    virtual void initialize();
    virtual void advance(double computedTimeStepSize);
    virtual void finalize();

    // Status queries
    virtual bool isCouplingOngoing() const;
    virtual bool requiresInitialData() const;
    virtual bool requiresWritingCheckpoint() const;
    virtual bool requiresReadingCheckpoint() const;

    virtual double getMaxTimeStepSize() const;

    // Profiling
    virtual void startProfilingSection(const std::string& name);
    virtual void stopLastProfilingSection();

private:
    std::string _participantName;
    std::string _configFileName;
    int _rank;
    int _size;
    bool _isInitialized;
    int _timeWindowCount;

};
}
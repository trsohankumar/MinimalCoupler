#include <string>

namespace precice
{
class Participant
{
    Participant(
      std::string participantName,
      std::string configurationFileName,
      int         solverProcessIndex,
      int         solverProcessSize);


    ~Participant    () = default;

    void initialize ();
    void advance    (double computedTimeStepSize);
    void finalize   ();
};
}
#pragma once
#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <utility>
#include "BasicBreakpointManager.h"

class cmCommand;
class BasicIncomingSocket;
class cmListFileFunction;
enum class MessageType;

namespace Sysprogs
{
	class ReplyBuilder;
	class RequestReader;
	enum class HLDPPacketType;
	enum class TargetStopReason;
	enum class ExpressionType;

	class HLDPServer
	{
	public:
		HLDPServer(int tcpPort, std::string debugTokenPath);
		~HLDPServer();

		typedef int UniqueScopeID, UniqueExpressionID;

		class RAIIScope
		{
		private:
			HLDPServer *m_pServer;
			UniqueScopeID m_UniqueID;

		public:
			cmMakefile *Makefile;
			const cmListFileFunction &Function;
			std::string SourceFile;
			cmStateDetail::PositionType Position;
			UniqueScopeID GetUniqueID() { return m_UniqueID; }

		public:
			RAIIScope(HLDPServer *pServer, cmMakefile *pMakefile, const cmListFileFunction &function);

			RAIIScope(const RAIIScope &) = delete;
			void operator=(const RAIIScope &) = delete;

			~RAIIScope();
		};

	public: // Public interface for debugged code
		bool WaitForClient();
		std::unique_ptr<RAIIScope> OnExecutingInitialPass(cmMakefile *pMakefile, const cmListFileFunction &function, bool &skipThisInstruction);
		void AdjustNextExecutedFunction(const std::vector<cmListFileFunction> &functions, size_t &i);

		void OnMessageProduced(MessageType type, const std::string &message);
		void OnVariableAccessed(const std::string &variable, int access_type, const char *newValue, const cmMakefile *mf);
		void OnTargetCreated(cmStateEnums::TargetType type, const std::string &targetName);

	private:
		void HandleBreakpointRelatedCommand(HLDPPacketType type, RequestReader &reader);
		bool SendReply(HLDPPacketType packetType, const ReplyBuilder &builder);
		HLDPPacketType ReceiveRequest(RequestReader &reader); // Returns 'Invalid' on error
		void SendErrorPacket(std::string details);

		void ReportStopAndServeDebugRequests(TargetStopReason stopReason, unsigned intParam, const std::string &stringParam, bool *pSkipThisInstruction);
		bool IsIgnoreVarWrite(const std::string &variable);

	private:
		class ExpressionBase
		{
		public:
			UniqueExpressionID AssignedID = -1;
			std::string Name, Value, Type;
			ExpressionType ExprType;
			int ExprSubType = 0;
			int ChildCountOrMinusOneIfNotYetComputed = 0;

		public:
			std::vector<UniqueExpressionID> RegisteredChildren;
			bool ChildrenRegistered = false;
			virtual std::vector<std::unique_ptr<ExpressionBase>> CreateChildren() { return std::vector<std::unique_ptr<ExpressionBase>>(); }

			virtual bool UpdateValue(const std::string &value, std::string &error)
			{
				error = "This expression cannot be edited.";
				return false;
			}

		public:
			virtual ~ExpressionBase() {}
		};

		class SimpleExpression;
		class VariableExpression;
		class CacheEntryExpression;
		class EnvironmentVariableExpression;
		class TargetExpression;
		class CacheTxtExpression;
		class TargetsExpression;
		class TargetPropertyListExpression;
		class EnvironmentMetaExpression;

		class DomainSpecificBreakpoint;

	private:
		std::unique_ptr<ExpressionBase> CreateExpression(const std::string &text, const RAIIScope &scope);

	private:
		BasicIncomingSocket *m_pSocket;
		std::string m_DebugToken;
		bool m_BreakInPending = false;
		bool m_EventsReported = false;
		bool m_Detached = false;
		int m_NextOneBasedLineToExecute = 0; // Used with "Set next statement"

		std::vector<RAIIScope *> m_CallStack;
		std::map<UniqueExpressionID, std::unique_ptr<ExpressionBase>> m_ExpressionCache;
		BasicBreakpointManager m_BreakpointManager;

		//Set of variables that ever had watches created. This should reduce the delay when checking each variable access.
		std::set<BasicBreakpointManager::CaseInsensitiveObjectName> m_WatchedVariables;

		// file -> var name -> line where the var was last written
		std::unordered_map<std::string, std::map<std::string, int>> m_VarLines;
		// file -> line -> (var name, var value)+
		std::unordered_map<std::string, std::multimap<int, std::pair<std::string, std::string>>> m_VarValues;

		// Set of command names for which we want to track variable writes.
		// Includes only builtin commands to filter out writes to parameters
		// of user-defined functions.
		std::unordered_set<std::string> m_VarWriteCommands;
		bool m_VarWriteCommandsInitialized = false;

		// Holds names of all created targets
		std::set<std::string> m_TargetNames;
		enum
		{
			kNoScope = -1,
			kRootScope = -2,
		};

		UniqueScopeID m_NextScopeID = 0;
		UniqueScopeID m_EndOfStepScopeID = kNoScope;

		UniqueExpressionID m_NextExpressionID = 0;
	};
} // namespace Sysprogs

#ifndef CLANG_RUNTIME_SPECIALIZER_JITPIPELINEIPSCCP_H
#define CLANG_RUNTIME_SPECIALIZER_JITPIPELINEIPSCCP_H

#include "JITPipeline.h"

namespace clangRuntimeSpecializer {

/// Pipeline 2: JIT-IPSCCP specialization pipeline.
///
/// Runs linkage scrub → early GlobalDCE → JitIPSCCPPass (with function
/// specialization enabled) → GlobalDCE → AlwaysInliner → cleanup.
/// No outer fixpoint loop: the SCCP solver converges internally.
/// The solver extends LLVM's IPSCCP with JIT-specific !invariant.load
/// resolution and vtable indirect-call devirtualization.
llvm::Error runIPSCCPPipeline(PipelineRunArgs& Args);

} // namespace clangRuntimeSpecializer

#endif // CLANG_RUNTIME_SPECIALIZER_JITPIPELINEIPSCCP_H

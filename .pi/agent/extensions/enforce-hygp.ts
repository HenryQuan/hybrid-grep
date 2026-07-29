import type { ExtensionAPI } from "@earendil-works/pi-coding-agent";

const BLOCKED_TOOLS = new Set(["read", "grep", "find", "ls"]);

export default function (pi: ExtensionAPI) {
  pi.on("session_start", async (_event, ctx) => {
    // Remove blocked tools from active set. Model cannot call what isn't listed.
    const allTools = pi.getAllTools().map((t) => t.name);
    const filtered = allTools.filter((name) => !BLOCKED_TOOLS.has(name));
    pi.setActiveTools(filtered);
  });

  // Safety net: block any blocked tool call that sneaks through
  pi.on("tool_call", async (event, ctx) => {
    if (BLOCKED_TOOLS.has(event.toolName)) {
      ctx.ui.notify(`${event.toolName} tool blocked — use hygp instead`, "error");
      return {
        block: true,
        reason: `\`${event.toolName}\` tool is disabled. Use \`hygp\` commands for file operations.`,
      };
    }
  });
}

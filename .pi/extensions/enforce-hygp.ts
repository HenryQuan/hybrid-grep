import type { ExtensionAPI } from "@earendil-works/pi-coding-agent";

const BLOCKED_TOOLS = new Set(["read", "grep", "find", "ls"]);

export default function (pi: ExtensionAPI) {
  pi.on("session_start", async () => {
    const allTools = pi.getAllTools().map((t) => t.name);
    const filtered = allTools.filter((name) => !BLOCKED_TOOLS.has(name));
    pi.setActiveTools(filtered);
  });

  pi.on("tool_call", async (event, ctx) => {
    if (BLOCKED_TOOLS.has(event.toolName)) {
      ctx.ui.notify(`${event.toolName} blocked — use hygp instead`, "error");
      return {
        block: true,
        reason: `\`${event.toolName}\` tool is disabled. Use \`hygp\` commands:\n  hygp rg <pattern>    — search\n  hygp fd <path>       — find files\n  hygp read|cat <file> — capped read\n  hygp sed <file> <n> [<m>]  — exact lines`,
      };
    }
  });
}

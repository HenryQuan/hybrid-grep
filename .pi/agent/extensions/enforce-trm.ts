import type { ExtensionAPI } from "@earendil-works/pi-coding-agent";

// Remove token-hungry tools; every bash command runs through trim (output capped).
const BLOCKED_TOOLS = new Set(["read", "grep", "find", "ls"]);

export default function (pi: ExtensionAPI) {
  pi.on("session_start", async () => {
    const allTools = pi.getAllTools().map((t) => t.name);
    const filtered = allTools.filter((name) => !BLOCKED_TOOLS.has(name));
    pi.setActiveTools(filtered);
  });

  pi.on("tool_call", async (event) => {
    if (event.toolName !== "bash") return;
    const cmd = event.input.command;
    if (typeof cmd === "string" && !cmd.match(/^\s*trim(\s|$)/)) {
      event.input.command = `trim ${cmd}`;
    }
  });
}

import type { ExtensionAPI } from "@earendil-works/pi-coding-agent";

// Every bash command runs through trim: output is capped, nothing is blocked.
export default function (pi: ExtensionAPI) {
  pi.on("tool_call", async (event) => {
    if (event.toolName !== "bash") return;
    const cmd = event.input.command;
    if (typeof cmd === "string" && !cmd.match(/^\s*trim(\s|$)/)) {
      event.input.command = `trim ${cmd}`;
    }
  });
}

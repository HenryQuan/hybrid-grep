import type { ExtensionAPI } from "@earendil-works/pi-coding-agent";

// Enforce henry-guide 1.2 mechanically: if the agent streams self-doubt
// (hesitation words) instead of stopping to ask, abort the turn and hand
// control back to the user. Tune the marker list below.

const CONFUSION_MARKERS = [
  "i'm not sure",
  "i am not sure",
  "not sure whether",
  "not sure if",
  "unsure",
  "uncertain",
  "probably wrong",
  "might be wrong",
  "could be wrong",
  "am i wrong",
  "i may be wrong",
  "i'm confused",
  "i am confused",
  "not certain",
  "i don't know",
];

export default function (pi: ExtensionAPI) {
  let abortedThisTurn = false;

  pi.on("turn_start", () => {
    abortedThisTurn = false;
  });

  pi.on("message_update", async (event, ctx) => {
    if (event.message?.role !== "assistant") return;
    if (abortedThisTurn) return;

    const content = event.message.content;
    const text =
      typeof content === "string"
        ? content
        : JSON.stringify(content ?? "");

    // Ignore code blocks: doubt inside code quotes is not self-doubt.
    const plain = text.replace(/```[\s\S]*?```/g, " ").toLowerCase();

    const hit = CONFUSION_MARKERS.find((m) => plain.includes(m));
    if (!hit) return;

    abortedThisTurn = true;
    await ctx.abort();
    ctx.ui.notify(
      `Agent seems unsure (matched: "${hit}") — please clarify the direction.`,
      "error"
    );
  });
}

const filters = document.querySelectorAll(".filter-button");
const eventCards = [...document.querySelectorAll(".event-card")];
const searchInput = document.querySelector("#event-search");
const noResults = document.querySelector("#no-results");
const modal = document.querySelector("#registration-modal");
const modalEventName = document.querySelector("#modal-event-name");
const eventIdInput = document.querySelector("#event-id");
const registrationForm = document.querySelector("#registration-form");
const formMessage = document.querySelector("#form-message");
const toast = document.querySelector("#success-toast");
let activeFilter = "all";
let lastFocusedElement = null;
const apiBaseUrlPromise = fetch(new URL("config.json", document.baseURI), { cache: "no-store" })
  .then(async (response) => {
    if (!response.ok) throw new Error("The event registration service is not configured yet.");
    const config = await response.json();
    if (typeof config.apiBaseUrl !== "string") {
      throw new Error("The event registration service configuration is invalid.");
    }
    return config.apiBaseUrl.trim().replace(/\/+$/, "");
  });

function updateEvents() {
  const query = searchInput.value.trim().toLowerCase();
  let visibleCount = 0;

  for (const card of eventCards) {
    const matchesCategory = activeFilter === "all" || card.dataset.category === activeFilter;
    const matchesSearch = !query || `${card.dataset.search} ${card.textContent}`.toLowerCase().includes(query);
    card.hidden = !(matchesCategory && matchesSearch);
    if (!card.hidden) visibleCount += 1;
  }

  noResults.hidden = visibleCount !== 0;
}

filters.forEach((button) => {
  button.addEventListener("click", () => {
    activeFilter = button.dataset.filter;
    filters.forEach((filter) => {
      const selected = filter === button;
      filter.classList.toggle("is-active", selected);
      filter.setAttribute("aria-pressed", String(selected));
    });
    updateEvents();
  });
});

searchInput.addEventListener("input", updateEvents);

function closeModal() {
  modal.hidden = true;
  if (lastFocusedElement) lastFocusedElement.focus();
}

document.querySelectorAll(".register-button").forEach((button) => {
  button.addEventListener("click", () => {
    lastFocusedElement = button;
    modalEventName.textContent = button.dataset.event;
    eventIdInput.value = button.dataset.eventId;
    formMessage.textContent = "";
    modal.hidden = false;
    document.querySelector("#student-name").focus();
  });
});

document.querySelector(".modal-close").addEventListener("click", closeModal);
modal.addEventListener("click", (event) => {
  if (event.target === modal) closeModal();
});
document.addEventListener("keydown", (event) => {
  if (event.key === "Escape" && !modal.hidden) closeModal();
  if (event.key === "/" && document.activeElement !== searchInput && modal.hidden) {
    event.preventDefault();
    searchInput.focus();
  }
});

registrationForm.addEventListener("submit", async (event) => {
  event.preventDefault();
  formMessage.textContent = "";

  const submitButton = registrationForm.querySelector('button[type="submit"]');
  submitButton.disabled = true;
  submitButton.textContent = "Saving your spot...";

  const formData = new FormData(registrationForm);
  const registration = Object.fromEntries(formData.entries());

  try {
    const apiBaseUrl = await apiBaseUrlPromise;
    const registrationUrl = apiBaseUrl ? `${apiBaseUrl}/api/register` : "/api/register";
    const response = await fetch(registrationUrl, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(registration),
    });
    const result = await response.json();

    if (!response.ok) {
      throw new Error(result.error || "We couldn't save your registration. Please try again.");
    }

    registrationForm.reset();
    closeModal();
    toast.hidden = false;
    window.setTimeout(() => { toast.hidden = true; }, 4200);
  } catch (error) {
    formMessage.textContent = error instanceof Error
      ? error.message
      : "We couldn't reach the registration server. Please try again.";
  } finally {
    submitButton.disabled = false;
    submitButton.innerHTML = 'Count me in <span aria-hidden="true">↗</span>';
  }
});

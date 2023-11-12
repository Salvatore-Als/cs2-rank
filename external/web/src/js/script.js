const themeIcon = document.getElementById('theme-icon');
const localStorageKey = 'cs2ranktheme';

let currentPage = 1;
let totalPage = 0;

if (localStorage[localStorageKey] == 'dark') {
    document.documentElement.classList.add('dark');
}

switchIcon();
getPlayers();

function nextPage() {
    if (currentPage + 1 > totalPage) {
        return;
    }

    currentPage++;
    getPlayers();
}

function previousPage() {
    if (currentPage - 1 <= 0) {
        return;
    }

    currentPage--;
    getPlayers();
}

function lazyProfileImage() {
    const images = document.getElementsByName('steam-picture');

    if (!Array.from(images)?.length) {
        return;
    }

    for (let image of Array.from(images)) {
        $.ajax({
            url: 'getAvatar.php',
            method: 'GET',
            data: { authid: image.getAttribute('data-authid') },
            success: function (response) {
                image.setAttribute('src', JSON.parse(JSON.stringify(response)));
            }
        });
    }
}

function getPlayers() {
    const playersTable = document.getElementById('players-tables');

    if (!playersTable) {
        return;
    }

    playersTable.innerHTML = "";

    for (let i = 0; i <= 5; i++) {
        playersTable.innerHTML = playersTable.innerHTML + generatePlaceholder();
    }

    $.ajax({
        url: 'getRank.php',
        method: 'GET',
        data: { page: currentPage },
        success: function (response) {
            playersTable.innerHTML = "";

            const result = JSON.parse(JSON.stringify(response));

            totalPage = result.totalPage;
            const players = result?.results;

            for (let player of players) {
                playersTable.innerHTML = playersTable.innerHTML + generatePlayerRow(player);
            }

            lazyProfileImage();
            updatePagination();
        }
    });
}

function updatePagination() {
    const currentPageSpan = document.getElementById('currentPage');
    if (currentPageSpan) {
        currentPageSpan.innerHTML = currentPage;
    }

    const previousPageButton = document.getElementById('previousPage');
    if (previousPageButton) {
        previousPageButton.disabled = currentPage <= 1;
    }

    const nextPageButton = document.getElementById('nextPage');
    if (nextPageButton) {
        nextPageButton.disabled = currentPage >= totalPage || totalPage <= 1;
    }
}

function generatePlaceholder() {
    return `
    <tr class="bg-white hover:bg-gray-50 dark:bg-gray-800 dark:border-gray-700 dark:hover:bg-gray-600">
        <td class="px-6 py-4 animate-pulse">
            <div class="h-[5px] bg-gray-200 rounded-full dark:bg-gray-700 w-48 mb-4"></div>
        </td>
        <td class="px-6 py-4 animate-pulse">
            <div class="h-[5px] bg-gray-200 rounded-full dark:bg-gray-700 w-48 mb-4"></div>
        </td>
        <td class="px-6 py-4 animate-pulse">
            <div class="h-[5px] bg-gray-200 rounded-full dark:bg-gray-700 w-48 mb-4"></div>
        </td>
        <td class="px-6 py-4 animate-pulse">
            <div class="h-[5px] bg-gray-200 rounded-full dark:bg-gray-700 w-48 mb-4"></div>
        </td>
        <td class="px-6 py-4 animate-pulse">
            <div class="h-[5px] bg-gray-200 rounded-full dark:bg-gray-700 w-48 mb-4"></div>
        </td>
        <td class="px-6 py-4 animate-pulse">
            <div class="h-[5px] bg-gray-200 rounded-full dark:bg-gray-700 w-48 mb-4"></div> 
        </td>
        <td class="px-6 py-4 animate-pulse">
            <div class="h-[5px] bg-gray-200 rounded-full dark:bg-gray-700 w-48 mb-4"></div>
        </td>
    </tr>
    `;
}

function generatePlayerRow(player) {
    return `
    <tr class="bg-white hover:bg-gray-50 dark:bg-gray-800 dark:border-gray-700 dark:hover:bg-gray-600">
        <td class="px-6 py-4 font-normal text-gray-900 dark:text-white">
            ${player.metadata?.rank}
        </td>
        <td class="px-6 py-4 font-normal text-gray-900 dark:text-white">
            <div class="flex gap-2">
                <div class="relative h-10 w-10">
                    <img class="h-full w-full rounded-full object-cover object-center" name="steam-picture" loading="lazy" data-authid="${player.authid}" src="https://avatars.cloudflare.steamstatic.com/fef49e7fa7e1997310d705b2a6158ff8dc1cdfeb_full.jpg" />
                </div>

                <div class="text-sm flex flex-col">
                    <a href="https://steamcommunity.com/profiles/${player.authid}" class="font-medium dark:text-gray-300 text-gray-700">
                        ${player.name}
                    </a>
                    <span class="text-gray-400">
                        ${new Date(player.lastconnect).toLocaleString()}
                    </span>
                </div>
            </div>
        </td>
        <td class="px-6 py-4">
            <span class="font-medium dark:text-gray-300 text-gray-700">
                ${player.points}
            </span>
        </td>
        <td class="px-6 py-4">
            <div class="flex flex-col gap-1">
                <div class="flex gap-2">
                    <span class="inline-flex items-center font-medium dark:text-gray-300 text-gray-700">
                        ${player.metadata?.kill?.total}
                    </span>
                    <span class="inline-flex items-center rounded-md dark:bg-red-700 bg-red-50 px-2 py-1 text-xs font-medium dark:text-red-50 text-red-700 ring-1 ring-inset dark:ring-red-600/10 ring-red-600/10">
                        ${player.metadata?.kill?.t}% T    
                    </span>
                    <span class="inline-flex items-center rounded-md dark:bg-blue-700 bg-blue-50 px-2 py-1 text-xs font-medium dark:text-blue-50 text-blue-700 ring-1 ring-inset dark:ring-blue-700/10 ring-blue-700/10">
                    ${player.metadata?.kill?.t}% CT 
                    </span>
                </div>
                <div class="flex gap-2">
                    <span class="inline-flex items-center rounded-md dark:bg-gray-700 bg-gray-50 px-2 py-1 text-xs font-medium dark:text-gray-50 text-gray-600 ring-1 ring-inset dark:ring-gray-500/10 ring-gray-500/10">
                        ${player.metadata?.kill?.headshot}% Headshot
                    </span>
                    <span class="inline-flex items-center rounded-md dark:bg-gray-700 bg-gray-50 px-2 py-1 text-xs font-medium dark:text-gray-50 text-gray-600 ring-1 ring-inset dark:ring-gray-500/10 ring-gray-500/10">
                        ${player.metadata?.kill?.knife}% Knife
                    </span>
                </div>
            </div>
        </td>
        <td class="px-6 py-4 flex gap-2">
            <span class="inline-flex items-center font-medium dark:text-gray-300 text-gray-700">
                ${player.metadata?.killassist?.total}
            </span>
            <span class="inline-flex items-center rounded-md dark:bg-red-700 bg-red-50 px-2 py-1 text-xs font-medium dark:text-red-50 text-red-700 ring-1 ring-inset dark:ring-red-600/10 ring-red-600/10"">
                ${player.metadata?.killassist?.t}% T    
            </span>
            <span class="inline-flex items-center rounded-md dark:bg-blue-700 bg-blue-50 px-2 py-1 text-xs font-medium dark:text-blue-50 text-blue-700 ring-1 ring-inset dark:ring-blue-700/10 ring-blue-700/10">
                ${player.metadata?.killassist?.ct}% CT
            </span>
        </td>
        <td class="px-6 py-4">
            <div class="flex flex-col gap-1">
                <div class="flex gap-1">
                    <span class="inline-flex items-center font-medium dark:text-gray-300 text-gray-700">
                        ${player.metadata?.death?.total}
                    </span>
                    <span class="inline-flex items-center rounded-md dark:bg-red-700 bg-red-50 px-2 py-1 text-xs font-medium dark:text-red-50 text-red-700 ring-1 ring-inset dark:ring-red-600/10 ring-red-600/10"">
                        ${player.metadata?.death?.t}% T
                    </span>
                    <span class="inline-flex items-center rounded-md dark:bg-blue-700 bg-blue-50 px-2 py-1 text-xs font-medium dark:text-blue-50 text-blue-700 ring-1 ring-inset dark:ring-blue-700/10 ring-blue-700/10">
                        ${player.metadata?.death?.ct}% CT
                    </span>
                </div>
                <div class="flex gap-1">
                    <span class="inline-flex items-center rounded-md dark:bg-gray-700 bg-gray-50 px-2 py-1 text-xs font-medium dark:text-gray-50 text-gray-600 ring-1 ring-inset dark:ring-gray-500/10 ring-gray-500/10">
                        ${player.metadata?.death?.suicide}% Suicide
                    </span>
                </div>
            </div>
        </td>
        <td class="px-6 py-4">
            <div class="flex gap-2">
                <span class="inline-flex items-center rounded-md dark:bg-gray-700 bg-gray-50 px-2 py-1 text-xs font-medium dark:text-gray-50 text-gray-600 ring-1 ring-inset dark:ring-gray-500/10 ring-gray-500/10">
                ${player.bomb_planted}% planted; 
                </span>
                <span class="inline-flex items-center rounded-md dark:bg-gray-700 bg-gray-50 px-2 py-1 text-xs font-medium dark:text-gray-50 text-gray-600 ring-1 ring-inset dark:ring-gray-500/10 ring-gray-500/10">
                ${player.bomb_exploded}% exploded; 
                </span>
                <span class="inline-flex items-center rounded-md dark:bg-gray-700 bg-gray-50 px-2 py-1 text-xs font-medium dark:text-gray-50 text-gray-600 ring-1 ring-inset dark:ring-gray-500/10 ring-gray-500/10">
                    <?php echo $player['bomb_defused']; ?> defused
                    ${player.bomb_defused}% defused; 
                </span>
            </div>
        </td>
    </tr>
    `;
}

function switchIcon() {
    themeIcon.classList = localStorage[localStorageKey] == 'dark' ? 'fa-solid fa-sun' : 'fa-solid fa-moon'
}

function changeTheme() {
    if (localStorage[localStorageKey] == 'dark') {
        localStorage.removeItem(localStorageKey);
        document.documentElement.classList.remove('dark');
    } else {
        localStorage[localStorageKey] = 'dark';
        document.documentElement.classList.add('dark');
    }

    switchIcon();
}